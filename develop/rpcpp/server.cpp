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

// --

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

// --

void f21_pair(int i, const std::string &s, float f) {
  std::cout << "f21_pair: i = " << i << ", s = " << s << ", f = " << f
            << std::endl;
}

void f22_pair(float f, const std::string &s) {
  std::cout << "f22_pair: f = " << f << ", s = " << s << std::endl;
}

} // namespace

void case1_push(const std::string &url) {
  CaseOutput co(__func__);

  rpcpp::PushNode<std::string> pn;
  pn.listen((url + ".Pipeline").c_str());

  for (int i = 0; i < 20; ++i) {
    if (i % 2 == 0)
      pn.transmit("f1", 23, std::string("from MyPush"), 2.333);
    if (i % 5 == 0)
      pn.transmit("f2", "from MyPush too.", 1.333);
    if (i % 7 == 0)
      pn.transmit("f3", 111, 1.111);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
}

std::future<int> case2_pushTask(const std::string &url) {
  CaseOutput co(__func__);

  auto node = std::make_unique<rpcpp::PushNode<std::string>>();
  node->listen((url + ".Pipeline").c_str());

  std::string name = "pushtask";
  auto task =
      std::make_unique<rpcpp::PushTask<std::string>>(name, std::move(node));
  std::future<int> f = g_poolServer->commit(std::ref(*task), 100);
  g_omServer->add(std::move(task));

  return f;
}

void post_push(const std::string &n) {
  auto t = (rpcpp::PushTask<std::string> *)g_omServer->getObjectPointer(n);
  for (int i = 0; i < 20; ++i) {
    if (i % 2 == 0)
      t->post("f1", 23, std::string("from MyPush"), 2.333);
    if (i % 5 == 0)
      t->post("f2", "from MyPush too.", 1.333);
    if (i % 7 == 0)
      t->post("f3", 111, 1.111);

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
}

int case3_reply(const std::string &url) {
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

std::future<int> case4_replyTask(const std::string &url) {
  CaseOutput co(__func__);

  auto e1 = std::make_unique<Exam1>("e1");
  g_omServer->add(std::move(e1));

  auto r = std::make_unique<rpcpp::Replier<std::string>>();
  r->defineClass("Ex").defineMethod("f", &Exam1::f33);
  r->defineFun("f1", f1).defineFun("f2", f2).defineFun("getObject", getObject);
  r->show();

  auto node = std::make_unique<rpcpp::ReplyNode<std::string>>(std::move(r));
  node->listen((url + ".RepReq").c_str());

  auto task = std::make_unique<rpcpp::ReplyTask<std::string>>("replytask",
                                                              std::move(node));
  std::future<int> f = g_poolServer->commit(std::ref(*task));
  g_omServer->add(std::move(task));

  return f;
}

int case5_publish(const std::string &url) {
  CaseOutput co(__func__);

  rpcpp::PublishNode<int> pn;
  pn.listen((url + ".PubSub").c_str());

  for (int i = 0; i < 30; ++i) {
    if (i % 2 == 0)
      pn.transmit(1, 101, std::string("publish message for int"), 1.01f);
    if (i % 3 == 0)
      pn.transmit(2, 202, std::string("p m 2"));
    if (i % 5 == 0)
      pn.transmit(3, (double)3.3);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  return 0;
}

std::future<int> case6_publishTask(const std::string &url) {
  CaseOutput co(__func__);

  auto node = std::make_unique<rpcpp::PublishNode<int>>();
  node->listen((url + ".PubSub").c_str());

  std::string name("publishtask");
  auto t = std::make_unique<rpcpp::PublishTask<int>>(name, std::move(node));
  std::future<int> f = g_poolServer->commit(std::ref(*t), 100);
  g_omServer->add(std::move(t));

  return f;
}

void post_publish(const std::string &n) {
  auto p = (rpcpp::PublishTask<int> *)g_omServer->getObjectPointer(n);
  for (int i = 0; i < 30; ++i) {
    if (i % 2 == 0)
      p->post(1, 101, std::string("publish message for int"), 1.01f);
    if (i % 3 == 0)
      p->post(2, 202, std::string("p m 2"));
    if (i % 5 == 0)
      p->post(3, (double)3.3);

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
}

int case7_pairServer(const std::string &url) {
  CaseOutput(__func__);

  auto r = std::make_unique<rpcpp::Resolver<int>>();
  r->defineFun(21, f21_pair).defineFun(22, f22_pair);

  auto node = std::make_unique<rpcpp::PairNode<int>>(std::move(r));
  node->listen((url + ".Pair").c_str());

  for (int i = 0; i < 50; ++i) {
    node->transact();
    if (i % 3 == 0)
      node->transmit(10, 23, std::string("from server of pair, 10"), 2.3f);

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  return 0;
}

std::future<int> case8_pairServerTask(const std::string &url) {
  CaseOutput co(__func__);

  auto r = std::make_unique<rpcpp::Resolver<int>>();
  r->defineFun(21, f21_pair).defineFun(22, f22_pair);

  auto node = std::make_unique<rpcpp::PairNode<int>>(std::move(r));
  node->listen((url + ".Pair").c_str());

  std::string name("pairServerTask");
  auto t = std::make_unique<rpcpp::PairTask<int>>(name, std::move(node));
  std::future<int> f = g_poolServer->commit(std::ref(*t), 100);
  g_omServer->add(std::move(t));

  return f;
}

void post_pairServer(const std::string &n) {
  // 向任务中post信息
  auto p = (rpcpp::PairTask<int> *)g_omServer->getObjectPointer(n);
  for (int i = 0; i < 50; ++i) {
    if (i % 3 == 0)
      p->post(10, 23, std::string("from server of pair, 10"), 2.3f);

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
}

// --

void f1_survey(const std::string s, rpcpp::UnsignedLong l, rpcpp::Double d) {
  std::cout << "f1_survey: s = " << s << ", l = " << l << ", d = " << d << std::endl;
}

void f2_survey(int i, rpcpp::Long l, rpcpp::Float f) {
  std::cout << "f2_survey: i = " << i << ", l = " << l << ", f = " << f << std::endl;
}

std::future<int> case10_surveyTask(const std::string &url) {
  CaseOutput co(__func__);

  auto r = std::make_unique<rpcpp::Resolver<int>>();
  r->defineFun(1, f1_survey).defineFun(2, f2_survey);

  auto node = std::make_unique<rpcpp::SurveyNode<int>>(std::move(r), 200);
  node->listen((url + ".SurRes").c_str());

  std::string name("surveyTask");
  auto t = std::make_unique<rpcpp::SurveyTask<int>>(name, std::move(node));
  std::future<int> f = g_poolServer->commit(std::ref(*t), 100);
  g_omServer->add(std::move(t));

  return f;
}

void post_survey(const std::string &n) {
  auto p = (rpcpp::SurveyTask<int> *)g_omServer->getObjectPointer(n);
  for (int i = 0; i < 32; ++i) {
    if (i % 3 == 0)
      p->post(3, std::string("survey 3"), (double)8.888);
    if (i % 7 == 0)
      p->post(5, 6.6f, 18, std::string("survey 5"));

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
}
// --

int startServer(const Anyarg &opt) {
  std::string url = opt.get_value_str('u');
  LOG_INFO << "url: " << url;

  // 服务器的线程池
  rpcpp::ThreadPool pool(16);
  g_poolServer = &pool;

  // 服务器的对象管理器
  rpcpp::Manager om("s_om1");
  g_omServer = &om;

  // case1_push(url);
  // case3_reply(url);
  // case5_publish(url);
  // case7_pairServer(url);

  std::future<int> f1 = case2_pushTask(url);
  std::future<int> f2 = case4_replyTask(url);
  std::future<int> f3 = case6_publishTask(url);
  std::future<int> f4 = case8_pairServerTask(url);
  std::future<int> f5 = case10_surveyTask(url);

  g_poolServer->commit(post_push, "pushtask");
  g_poolServer->commit(post_publish, "publishtask");
  g_poolServer->commit(post_pairServer, "pairServerTask");
  g_poolServer->commit(post_survey, "surveyTask");

  std::cout << "wait for 50s" << std::endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(50000));

  om.close();
  pool.stop();

  std::cout << "f1 = " << f1.get() << std::endl;
  std::cout << "f2 = " << f2.get() << std::endl;
  std::cout << "f3 = " << f3.get() << std::endl;
  std::cout << "f4 = " << f4.get() << std::endl;
  std::cout << "f5 = " << f5.get() << std::endl;

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
