#include <functional>
#include <iostream>
#include <sstream>

#include <pp/prettyprint.h>

#include <nanolog/nanolog.hpp>

#include <msgpack-c/msgpack.hpp>

#include <nng/nng.h>

#include "caseoutput.h"
#include "anyarg.h"
#include "threadpool.h"
#include "rpcpp.h"
#include "resolver.h"
#include "node.h"
#include "nodeclient.h"
#include "task.h"
#include "taskclient.h"
#include "client.h"

// --

rpcpp::ThreadPool *g_poolClient = nullptr;
rpcpp::Manager *g_omClient = nullptr;

// --
namespace {

void ff1(int i, const std::string &s, double d) {
  std::cout << "recv tid: " << std::this_thread::get_id() << std::endl;
  std::cout << "ff1: " << i << ", " << s << ", " << d << std::endl;
}

void ff2(const std::string &s, float f) {
  std::cout << "recv2 tid: " << std::this_thread::get_id() << std::endl;
  std::cout << "ff2: " << s << ", " << f << std::endl;
}

void ff3(int i, double d) {
  std::cout << "recv3 tid: " << std::this_thread::get_id() << std::endl;
  std::cout << "ff3: " << i << ", " << d << std::endl;
}

void request_chunk(rpcpp::RequestNode<std::string> *n) {
  int i1;
  n->request(i1, std::string("f1"), 12, 23);
  std::cout << "f1(12, 13) = " << i1 << std::endl;

  std::string s;
  n->request(s, std::string("f2"), "goodboy");
  std::cout << "f2(goodboy) = " << s << std::endl;

  std::intptr_t oid;
  n->request(oid, std::string("getObject"), "e1");
  std::cout << "getObject(\"e1\") = " << oid << std::endl;

  int i2;
  n->requestOnObj(i2, std::string("Ex"), std::string("f"), oid, 29);
  std::cout << "Ex::f(29) = " << i2 << std::endl;
}

} // namespace

void case1_pull(const std::string &url) {
  CaseOutput co(__func__);

  auto p = std::make_unique<rpcpp::Resolver<std::string>>();
  p->defineFun("f1", ff1);
  p->defineFun("f2", ff2);
  p->defineFun("f3", ff3);
  rpcpp::PullNode<std::string> mp(std::move(p));

  mp.dial((url + ".Pipeline").c_str());
  mp.transact();
  mp.transact();
  mp.transact();
}

void case2_request(const std::string &url) {
  CaseOutput co(__func__);

  rpcpp::RequestNode<std::string> r;
  r.dial((url + ".RepReq").c_str());

  request_chunk(&r);
}

std::future<int> case3_requestTask(const std::string &url) {
  CaseOutput co(__func__);

  std::string n = "req1";
  std::future<int> f;
  auto node = std::make_unique<rpcpp::RequestNode<std::string>>();
  node->dial((url + ".RepReq").c_str());

  auto t = std::make_unique<rpcpp::RequestTask<std::string>>(n, std::move(node));
  f = g_poolClient->commit(std::ref(*t), 100);

  g_omClient->add(std::move(t));

  rpcpp::RequestTask<std::string> *p =
      (rpcpp::RequestTask<std::string> *)g_omClient->getObjectPointer(n);
  p->addChunk(request_chunk);

  return f;
}

// --

int startClient(const Anyarg &opt) {
  std::string url = opt.get_value_str('u');
  LOG_INFO << "url: " << url;

  // 全局线程池，在本函数退出时，析构pool时，会退出所有线程
  rpcpp::ThreadPool pool(6);
  g_poolClient = &pool;

  rpcpp::Manager om("c_om1");
  g_omClient = &om;

  case1_pull(url);
  case2_request(url);
  case3_requestTask(url);

  std::this_thread::sleep_for(std::chrono::milliseconds(10000));

  om.close();
  pool.stop();
  pool.join_all();

  return 0;
}

// --
