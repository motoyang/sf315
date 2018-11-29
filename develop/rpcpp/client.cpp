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

void f1_pull(int i, const std::string &s, double d) {
  std::cout << "f1_pull: " << i << ", " << s << ", " << d << std::endl;
}

void f2_pull(const std::string &s, float f) {
  std::cout << "f2_pull: " << s << ", " << f << std::endl;
}

void f3_pull(int i, double d) {
  std::cout << "f3_pull: " << i << ", " << d << std::endl;
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

void f1_sub(int i, const std::string &s, float f) {
  std::cout << "f1_sub: i = " << i << ", s = " << s << ", f = " << f
            << std::endl;
}

void f2_sub(int i, const std::string &s) {
  std::cout << "f2_sub: i = " << i << ", s = " << s << std::endl;
}

void f3_sub(double d) { std::cout << "f3_sub: d = " << d << std::endl; }

} // namespace

void case1_pull(const std::string &url) {
  CaseOutput co(__func__);

  auto p = std::make_unique<rpcpp::Resolver<std::string>>();
  p->defineFun("f1", f1_pull);
  p->defineFun("f2", f2_pull);
  p->defineFun("f3", f3_pull);
  rpcpp::PullNode<std::string> mp(std::move(p));

  mp.dial((url + ".Pipeline").c_str());
  mp.transact();
  mp.transact();
  mp.transact();
}

std::future<int> case2_pullTask(const std::string &url) {
  CaseOutput co(__func__);

  auto p = std::make_unique<rpcpp::Resolver<std::string>>();
  p->defineFun("f1", f1_pull).defineFun("f2", f2_pull).defineFun("f3", f3_pull);

  auto pn = std::make_unique<rpcpp::PullNode<std::string>>(std::move(p));
  pn->dial((url + ".Pipeline").c_str());

  auto t =
      std::make_unique<rpcpp::PullTask<std::string>>("pulltask", std::move(pn));
  std::future<int> f = g_poolClient->commit(std::ref(*t));
  g_omClient->add(std::move(t));

  return f;
}

void case3_request(const std::string &url) {
  CaseOutput co(__func__);

  rpcpp::RequestNode<std::string> r;
  r.dial((url + ".RepReq").c_str());

  request_chunk(&r);
}

std::future<int> case4_requestTask(const std::string &url) {
  CaseOutput co(__func__);

  std::future<int> f;
  auto node = std::make_unique<rpcpp::RequestNode<std::string>>();
  node->dial((url + ".RepReq").c_str());

  std::string n = "req1";
  auto t =
      std::make_unique<rpcpp::RequestTask<std::string>>(n, std::move(node));
  f = g_poolClient->commit(std::ref(*t), 100);

  g_omClient->add(std::move(t));

  rpcpp::RequestTask<std::string> *p =
      (rpcpp::RequestTask<std::string> *)g_omClient->getObjectPointer(n);
  p->load(request_chunk);

  return f;
}

void case5_subscribe(const std::string &url) {
  CaseOutput co(__func__);

  int tag1 = 1, tag2 = 2, tag3 = 3;
  auto r = std::make_unique<rpcpp::Resolver<int>>();
  r->defineFun(1, f1_sub).defineFun(tag2, f2_sub).defineFun(tag3, f3_sub);

  auto node = std::make_unique<rpcpp::SubscribeNode<int>>(std::move(r));
  node->dial((url + ".PubSub").c_str());

  node->transact();
  node->transact();
  node->transact();
}

std::future<int> case6_subscribeTask(const std::string &url) {
  CaseOutput co(__func__);

  auto r = std::make_unique<rpcpp::Resolver<int>>();
  r->defineFun(1, f1_sub).defineFun(2, f2_sub).defineFun(3, f3_sub);

  std::future<int> f;
  auto node = std::make_unique<rpcpp::SubscribeNode<int>>(std::move(r));
  node->dial((url + ".PubSub").c_str());

  std::string n = "sub1";
  auto t = std::make_unique<rpcpp::SubscribeTask<int>>(n, std::move(node));
  f = g_poolClient->commit(std::ref(*t));

  g_omClient->add(std::move(t));

  return f;
}

void f10_pair(int i, const std::string &s, float f) {
  std::cout << "f10_pair: i = " << i << ", s = " << s << ", f = " << f
            << std::endl;
}

void cast7_pairClient(const std::string &url) {
  CaseOutput(__func__);

  auto r = std::make_unique<rpcpp::Resolver<int>>();
  r->defineFun(10, f10_pair);
  auto node = std::make_unique<rpcpp::PairNode<int>>(std::move(r));
  node->dial((url + ".Pair").c_str());

  for (int i = 0; i < 50; ++i) {
    node->transact();
    if (i % 2 == 0)
      node->transmit(21, 3, std::string("from client of pair, 21"), 38.8f);
    if (i % 5 == 0)
      node->transmit(22, 1.03f, std::string("from client, 22"));
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
}

std::future<int> case8_pairClientTask(const std::string &url) {
  CaseOutput co(__func__);

  auto r = std::make_unique<rpcpp::Resolver<int>>();
  r->defineFun(10, f10_pair);

  auto node = std::make_unique<rpcpp::PairNode<int>>(std::move(r));
  node->dial((url + ".Pair").c_str());

  std::string name("pairClientTask");
  auto t = std::make_unique<rpcpp::PairTask<int>>(name, std::move(node));
  std::future<int> f = g_poolClient->commit(std::ref(*t));
  g_omClient->add(std::move(t));

  return f;
}

void post_pairClient(const std::string &n) {
  // 向任务中post信息
  auto p = (rpcpp::PairTask<int> *)g_omClient->getObjectPointer(n);
  for (int i = 0; i < 30; ++i) {
    if (i % 2 == 0)
      p->post(21, 3, std::string("from client of pair, 21"), 38.8f);
    if (i % 5 == 0)
      p->post(22, 1.03f, std::string("from client, 22"));

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
}

// --

void f3_response(const std::string &s, double d) {
  std::cout << "f3_response: s = " << s << ", d = " << d << std::endl;
  auto p =
      (rpcpp::ResponseTask<int> *)g_omClient->getObjectPointer("responseTask");
  p->post(1, std::string("response for 3"), rpcpp::UnsignedLong(88888),
          rpcpp::Double(6.666));
}

void f5_response(float f, int i, const std::string &s) {
  std::cout << "f5_response: f = " << f << ", i = " << i << ", s = " << s
            << std::endl;
  auto p =
      (rpcpp::ResponseTask<int> *)g_omClient->getObjectPointer("responseTask");
  p->post(2, 222, rpcpp::Long(333333), rpcpp::Float(16.666));
}

std::future<int> case10_responseTask(const std::string &url) {
  CaseOutput co(__func__);

  auto r = std::make_unique<rpcpp::Resolver<int>>();
  r->defineFun(3, f3_response).defineFun(5, f5_response);

  auto node = std::make_unique<rpcpp::ResponseNode<int>>(std::move(r));
  node->dial((url + ".SurRes").c_str());

  std::string name("responseTask");
  auto t = std::make_unique<rpcpp::ResponseTask<int>>(name, std::move(node));
  std::future<int> f = g_poolClient->commit(std::ref(*t));
  // std::cout << "this3 = " << t.get() << std::endl;

  g_omClient->add(std::move(t));

  return f;
}

// --

void f1_bus(int i, const std::string &s, float f) {
  std::cout << "f1_bus: i = " << i << ", s = " << s << ", f = " << f
            << std::endl;
}

void f2_bus(double d, const std::string &s) {
  std::cout << "f2_bus: d = " << d << ", s = " << s << std::endl;
}

std::future<int> case12_busClientTask(const std::string &url) {
  CaseOutput co(__func__);

  auto r = std::make_unique<rpcpp::Resolver<int>>();
  r->defineFun(1, f1_bus).defineFun(2, f2_bus);

  auto node = std::make_unique<rpcpp::BusNode<int>>(std::move(r));
  node->dial((url + ".Bus").c_str());

  std::string name("busClientTask");
  auto t = std::make_unique<rpcpp::BusTask<int>>(name, std::move(node));
  std::future<int> f = g_poolClient->commit(std::ref(*t));
  g_omClient->add(std::move(t));

  return f;
}

void post_busClient(const std::string &n) {
  // 向任务中post信息
  auto p = (rpcpp::PairTask<int> *)g_omClient->getObjectPointer(n);
  for (int i = 0; i < 30; ++i) {
    if (i % 3 == 0)
      p->post(11, 3.3f, std::string("from client of bus, 11"), 33);
    if (i % 7 == 0)
      p->post(12, 103, std::string("from client of bus, 12"));

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
}

// --

int startClient(const Anyarg &opt) {
  std::string url = opt.get_value_str('u');
  LOG_INFO << "url: " << url;

  // 全局线程池，在本函数退出时，析构pool时，会退出所有线程
  rpcpp::ThreadPool pool(16);
  g_poolClient = &pool;

  rpcpp::Manager om("c_om1");
  g_omClient = &om;

  // case1_pull(url);
  // case3_request(url);
  // case5_subscribe(url);
  // cast7_pairClient(url);
  std::future<int> f1 = case2_pullTask(url);
  std::future<int> f2 = case4_requestTask(url);
  std::future<int> f3 = case6_subscribeTask(url);
  std::future<int> f4 = case8_pairClientTask(url);
  std::future<int> f5 = case10_responseTask(url);
  std::future<int> f6 = case12_busClientTask(url);

  g_poolClient->commit(post_pairClient, "pairClientTask");
  g_poolClient->commit(post_busClient, "busClientTask");

  std::cout << "wait for 50s" << std::endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(50000));

  om.close();
  pool.stop();

  std::cout << "f1 = " << f1.get() << std::endl;
  std::cout << "f2 = " << f2.get() << std::endl;
  std::cout << "f3 = " << f3.get() << std::endl;
  std::cout << "f4 = " << f4.get() << std::endl;
  std::cout << "f5 = " << f5.get() << std::endl;
  std::cout << "f6 = " << f6.get() << std::endl;

  pool.join_all();

  return 0;
}

// --
