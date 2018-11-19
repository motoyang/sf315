#include <string>
#include <iostream>
#include <sstream>

#include <nanolog/nanolog.hpp>

#include <msgpack-c/msgpack.hpp>

#include <nng/nng.h>
#include <nng/protocol/reqrep0/req.h>

#include "anyarg.h"
#include "threadpool.h"
#include "rpcpp.h"
#include "node.h"
#include "nodeclient.h"
#include "task.h"
#include "taskclient.h"
#include "client.h"

// --

rpcpp2::threadpool* g_poolClient = nullptr;
rpcpp2::TaskManager* g_tmClient = nullptr;

// --
namespace  {

void subscribe(const char* buf, size_t len) {
  msgpack::object_handle oh = msgpack::unpack(buf, len);
  std::string s = oh.get().as<std::string>();
  std::cout << "Received " << s.size() << " bytes:" << std::endl
            << s << std::endl;
}

std::future<int> commitSubscriber(const std::string& url) {
  auto p = std::make_unique<rpcpp2::client::SubscribeTask>(url + ".PubSub");
  p->init([]() {return true;});
  p->subscribe(subscribe);
  std::future<int> r = g_poolClient->commit(*p);
  g_tmClient->AddTask("Sub1", std::move(p));
  return r;
}

// --

int request(rpcpp2::client::RequestTask* rt) {
  std::string fn1("repFun1");
  std::string fn2("repFun2");
  std::string fn3("query");
  std::string fn4("quit");
  int result;
  rt->call(fn1, result, 55, std::string("client coming."));
  std::cout << "result: " << result << std::endl;
  rt->call(fn2, result, 66, std::string("client2 coming2."));
  std::cout << "result2: " << result << std::endl;
  std::string info;
  rt->call(fn3, info);
  std::cout << info << std::endl;

  using namespace std::chrono_literals;
  std::this_thread::sleep_for(5s);

  rt->call(fn3, info);
  std::cout << "222------" << std::endl
            << info << std::endl;
  rt->callAndClose(fn4);
  std::cout << "result4: quit" << std::endl;

  return 99;
}

std::future<int> commitRequester(const std::string& url) {
  auto p = std::make_unique<rpcpp2::client::RequestTask>(url + ".RepReq");
  p->request(request, p.get());
  std::future<int> r = g_poolClient->commit(*p);
  g_tmClient->AddTask("Req1", std::move(p));
  return r;
}

// --

std::string push(int i, std::string s, double d) {
  std::stringstream ss;
  ss << "i = " << i << ", s = " << s << ", d = " << d << std::endl;

  std::time_t t = std::time(nullptr);
  ss << std::put_time(std::localtime(&t), "%F %T");

  std::stringstream ss2;
  msgpack::pack(ss2, ss.str());

  return ss2.str();
}

void interval2() {
  using namespace std::chrono_literals;
  std::this_thread::sleep_for(1s);
}

std::future<int> commitPusher(const std::string& url) {
  auto p = std::make_unique<rpcpp2::PushTask>(url + ".P2C1");
  p->push(push, 128, "have a good day.", 8.88);
  p->interval(interval2);
  std::future r = g_poolClient->commit(*p);
  g_tmClient->AddTask("P2S1", std::move(p));
  return r;
}

// --

std::string say(int i, double d, const std::string s) {
  std::time_t t = std::time(nullptr);
  std::stringstream ss;
  ss << "Client say: i = " << i << ", d = " << d << ", s = " << s << std::endl;
  ss << std::put_time(std::localtime(&t), "%F %T");
  std::stringstream ss2;
  msgpack::pack(ss2, ss.str());
  return ss2.str();
}

void hear(const char* buf, size_t len) {
  msgpack::object_handle oh = msgpack::unpack(buf, len);
  std::string s = oh.get().as<std::string>();
  std::cout << "Client hear " << s.size() << " bytes:" << std::endl
            << s << std::endl;
}

void interval4() {
  using namespace std::chrono_literals;
  std::this_thread::sleep_for(1s);
}

std::future<int> commitPairClient(const std::string& url) {
  auto p = std::make_unique<rpcpp2::client::PairClientTask>(url + ".Pair");
  p->interval(interval4);
  p->hear(hear);
  p->say(say, 32, 6.6, "a good boy.");
  std::future<int> r = g_poolClient->commit(*p);
  g_tmClient->AddTask("Pair1", std::move(p));
  return r;
}

}

// --

int startClient(const Anyarg &opt) {
  int r = 0;
  std::string url = opt.get_value_str('u');

  // 全局线程池，在本函数退出时，析构pool时，会退出所有线程
  rpcpp2::threadpool pool(6);
  g_poolClient = &pool;

  rpcpp2::TaskManager tm;
  g_tmClient = &tm;

  std::future<int> f1 = commitSubscriber(url);
  std::future<int> f2 = commitRequester(url);
  std::future<int> f3 = commitPusher(url);
  std::future<int> f4 = commitPairClient(url);

  std::cout << "wait for 10s..." << std::endl;
  using namespace std::chrono_literals;
  std::this_thread::sleep_for(10s);
  std::cout << "quit." << std::endl;

  tm.stop();
  pool.stop();
  pool.join_all();

  std::cout << "f1 = " << f1.get() << std::endl;
  std::cout << "f2 = " << f2.get() << std::endl;
  std::cout << "f3 = " << f3.get() << std::endl;
  std::cout << "f4 = " << f4.get() << std::endl;

  return r;
}
