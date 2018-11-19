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
#include <nng/protocol/reqrep0/rep.h>
#include <nng/protocol/reqrep0/req.h>

#include "anyarg.h"
#include "threadpool.h"
#include "rpcpp.h"
#include "register.h"
#include "node.h"
#include "nodeserver.h"
#include "task.h"
#include "taskserver.h"
#include "server.h"

// --

rpcpp2::threadpool* g_poolServer = nullptr;
rpcpp2::TaskManager* g_tmServer = nullptr;

// --

std::string reply(const char *buf, size_t len) {
  std::size_t off = 0;
  msgpack::object_handle oh = msgpack::unpack(buf, len, off);
  std::string name;
  oh.get().convert(name);

  auto v = rpcpp2::server::Register::instance().find(name);
  auto f = v->first;

  std::string result = f(v->second, buf, len, off);

  return result;
}

std::future<int> commitResponder(const std::string& url) {
  auto p = std::make_unique<rpcpp2::server::ReplyTask>(url + ".RepReq");
  p->init(initRep, 33);
  p->reply(reply);
  std::future<int> r = g_poolServer->commit(*p);
  g_tmServer->AddTask("Rep1", std::move(p));
  return r;
}

// --

std::string publish(int i) {
  std::time_t t = std::time(nullptr);
  std::stringstream ss;
  ss << std::put_time(std::localtime(&t), "%F %T");
  std::stringstream ss2;
  msgpack::pack(ss2, ss.str());

  return ss2.str();
}

void interval() {
  using namespace std::chrono_literals;
  std::this_thread::sleep_for(2s);
}

std::future<int> commitPublisher(const std::string& url) {
  auto p = std::make_unique<rpcpp2::server::PublishTask>(url + ".PubSub");
  p->publish(publish, 23);
  p->interval(interval);
  std::future<int> r = g_poolServer->commit(*p);
  g_tmServer->AddTask("Pub1", std::move(p));
  return r;
}

// --

void pull(const char* buf, size_t len) {
  msgpack::object_handle oh = msgpack::unpack(buf, len);
  std::string s = oh.get().as<std::string>();
  std::cout << "Puller get " << s.size() << " bytes:" << std::endl
            << s << std::endl;
}

std::future<int> commitPuller(const std::string& url) {
  auto p = std::make_unique<rpcpp2::PullTask>(url + ".P2C1");
  p->init([](){return true;});
  p->pull(pull);
  std::future<int> r = g_poolServer->commit(*p);
  g_tmServer->AddTask("P2C1", std::move(p));
  return r;
}

// --

std::string say(int i, double d, const std::string s) {
  std::time_t t = std::time(nullptr);
  std::stringstream ss;
  ss << "Server say: i = " << i << ", d = " << d << ", s = " << s << std::endl;
  ss << std::put_time(std::localtime(&t), "%F %T");
  std::stringstream ss2;
  msgpack::pack(ss2, ss.str());
  return ss2.str();
}

void hear(const char* buf, size_t len) {
  msgpack::object_handle oh = msgpack::unpack(buf, len);
  std::string s = oh.get().as<std::string>();
  std::cout << "Server hear " << s.size() << " bytes:" << std::endl
            << s << std::endl;
}

void interval3() {
  using namespace std::chrono_literals;
  std::this_thread::sleep_for(2s);
}

std::future<int> commitPairServer(const std::string& url) {
  auto p = std::make_unique<rpcpp2::server::PairServerTask>(url + ".Pair");
  p->interval(interval3);
  p->hear(hear);
  p->say(say, 32, 6.6, "a good boy.");
  std::future<int> r = g_poolServer->commit(*p);
  g_tmServer->AddTask("Pair1", std::move(p));
  return r;
}

// --

int startServer(const Anyarg& opt) {
  std::string url = opt.get_value_str('u');
  LOG_INFO << "url getted: " << url;

  // 服务器的线程池
  rpcpp2::threadpool pool(6);
  g_poolServer = &pool;

  rpcpp2::TaskManager tm;
  g_tmServer = &tm;

  std::future<int> r1 = commitResponder(url);
  std::future<int> r2 = commitPublisher(url);
  std::future<int> r3 = commitPuller(url);
  std::future<int> r4 = commitPairServer(url);

//  pool.stop();
  pool.join_all();

  LOG_INFO << "Responder return: " << r1.get();
  LOG_INFO << "Publisher return: " << r2.get();
  LOG_INFO << "Puller return: " << r3.get();
  LOG_INFO << "PairServer return: " << r4.get();

  return 0;
}

int startDaemon(const Anyarg& opt) {
  if (-1 == daemon(0, 0)) {
    LOG_CRIT << "daemon error. errno: " << errno;
    return -1;
  }

  return startServer(opt);
}

// --
