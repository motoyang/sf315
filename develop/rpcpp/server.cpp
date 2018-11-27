#include <unistd.h>
#include <errno.h>

#include <functional>
#include <iostream>
#include <sstream>
#include <unordered_map>

#include <pp/prettyprint.h>

#include <nanolog/nanolog.hpp>

#include <msgpack-c/msgpack.hpp>

#include <nng/nng.h>

#include "caseoutput.h"
#include "anyarg.h"
#include "threadpool.h"
#include "rpcpp.h"
#include "objects.h"
#include "node.h"
#include "nodeserver.h"
#include "task.h"
#include "taskserver.h"
#include "server.h"

// --

rpcpp::ThreadPool *g_poolServer = nullptr;
rpcpp::Manager *g_omServer = nullptr;

// --

// 在匿名空间的函数仅仅在本文件中是可见的。
namespace {

class MyPush : public rpcpp::PushNode<std::string> {
public:
  void f1(int i, std::string const &s, double d) { transmit(__func__, i, s, d); }

  void f2(const char *s, float f) { transmit(__func__, s, f); }

  void f3(int i, double d) { transmit(__func__, i, d); }
};

int f1(int i, int j) { return i + j; }

std::string f2(const std::string &s) { return s + "_echo"; }

struct Exam1 : public rpcpp::Object {
  int _i = 13;
  Exam1(const std::string &n) : rpcpp::Object(n) {}
  int f33(int i) { return i + _i; }
};

rpcpp::Pointer getObject(const std::string &n) {
  return g_omServer->getObjectPointer(n);
}

} // namespace

void case1_push(const std::string &url) {
  CaseOutput co(__func__);

  MyPush mp;
  mp.listen((url + ".Pipeline").c_str());
  mp.f1(23, std::string("from MyPush"), 2.333);
  mp.f2("from MyPush too.", 1.333);
  mp.f3(111, 1.111);
}

int case2_reply(const std::string &url) {
  CaseOutput co(__func__);

  auto e1 = std::make_unique<Exam1>("e1");
  g_omServer->add(std::move(e1));

  auto r = std::make_unique<rpcpp::Replier<std::string>>();
  r->defineClass("Ex").defineMethod("fff", &Exam1::f33);
  r->defineFun("f1", f1).defineFun("f2", f2).defineFun("getObject", getObject);

  rpcpp::ReplyNode node(std::move(r));
  node.listen((url + ".RepReq").c_str());
  node.transact();
  node.transact();
  node.transact();
  node.transact();

  return 0;
}

std::future<int> case3_replyTask(const std::string &url) {
  CaseOutput co(__func__);

  auto e1 = std::make_unique<Exam1>("e1");
  g_omServer->add(std::move(e1));

  auto r = std::make_unique<rpcpp::Replier<std::string>>();
  r->defineClass("Ex").defineMethod("f", &Exam1::f33);
  r->defineFun("f1", f1).defineFun("f2", f2).defineFun("getObject", getObject);
  r->show();

  auto node = std::make_unique<rpcpp::ReplyNode<std::string>>(std::move(r));
  node->listen((url + ".RepReq").c_str());

  auto task =
      std::make_unique<rpcpp::ReplyTask<std::string>>("replytask", std::move(node));
  std::future<int> f = g_poolServer->commit(std::ref(*task));
  g_omServer->add(std::move(task));

  return f;
}

// --

int startServer(const Anyarg &opt) {
  std::string url = opt.get_value_str('u');
  LOG_INFO << "url: " << url;

  // 服务器的线程池
  rpcpp::ThreadPool pool(6);
  g_poolServer = &pool;

  // 服务器的对象管理器
  rpcpp::Manager om("s_om1");
  g_omServer = &om;

  std::future<int> f1 = case3_replyTask(url);

  case1_push(url);

  std::this_thread::sleep_for(std::chrono::milliseconds(5000));

  om.close();
  pool.stop();
  pool.join_all();

  return 0;
}

int startDaemon(const Anyarg &opt) {
  if (-1 == daemon(0, 0)) {
    LOG_CRIT << "daemon error. errno: " << errno;
    return -1;
  }

  return startServer(opt);
}

// --
