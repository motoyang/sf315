#include <string>
#include <iostream>
#include <sstream>

#include <nanolog/nanolog.hpp>

#include <msgpack.hpp>

#include <nng/nng.h>
#include <nng/protocol/reqrep0/req.h>

#include "anyarg.h"
#include "threadpool.h"
#include "node.h"
#include "nodeclient.h"
#include "task.h"
#include "taskclient.h"
#include "client.h"

// --

rpcpp2::threadpool* g_poolClient = nullptr;
rpcpp2::TaskManager* g_tmClient = nullptr;

// --

void subscribe(const char* buf, size_t len) {
  msgpack::object_handle oh = msgpack::unpack(buf, len);
  std::string s = oh.get().as<std::string>();
  std::cout << "Received " << s.size() << " bytes:" << std::endl
            << s << std::endl;
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
  std::this_thread::sleep_for(15s);

  rt->call(fn3, info);
  std::cout << "222------" << std::endl
            << info << std::endl;
  rt->callAndClose(fn4);
  std::cout << "result4: quit" << std::endl;

  return 99;
}

// --

int startClient(const Anyarg &opt) {
  int r = 0;
  std::string url = opt.get_value_str('u');

  // 全局线程池，在本函数退出时，析构pool时，会退出所有线程
  rpcpp2::threadpool pool(2);
  g_poolClient = &pool;

  rpcpp2::TaskManager tm;
  g_tmClient = &tm;

  auto p1 = std::make_unique<rpcpp2::client::SubscribeTask>(url + ".PubSub");
  p1->init([]() {return true;});
  p1->subscribe(subscribe);
  std::future<int> f1 = pool.commit(*p1);
  tm.AddTask("Sub1", std::move(p1));

  auto p2 = std::make_unique<rpcpp2::client::RequestTask>(url + ".RepReq");
  p2->request(request, p2.get());
  std::future<int> f2 = pool.commit(*p2);
  tm.AddTask("Req1", std::move(p2));

  std::cout << "wait for 20s..." << std::endl;
  using namespace std::chrono_literals;
  std::this_thread::sleep_for(20s);
  std::cout << "quit." << std::endl;

  tm.stop();
  pool.stop();
  pool.join_all();

  std::cout << "f1 = " << f1.get() <<  std::endl;
  std::cout << "f2 = " << f2.get() <<  std::endl;

  return r;
}
