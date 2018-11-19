#include <unistd.h>
#include <errno.h>

#include <functional>
#include <iostream>
#include <sstream>
#include <unordered_map>

#include <pp/prettyprint.h>

#include <nanolog/nanolog.hpp>

#include <msgpack.hpp>

#include <nng/nng.h>
#include <nng/protocol/reqrep0/rep.h>
#include <nng/protocol/reqrep0/req.h>

#include "register.h"
#include "anyarg.h"
#include "threadpool.h"
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

// --

std::string pub1(int i) {
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

// --

int startServer(const Anyarg& opt) {
  std::string url = opt.get_value_str('u');

  // 服务器的线程池
  rpcpp2::threadpool pool(6);
  g_poolServer = &pool;

  rpcpp2::TaskManager tm;
  g_tmServer = &tm;

  auto p1 = std::make_unique<rpcpp2::server::PublishTask>(url + ".PubSub");
  p1->publish(pub1, 23);
  p1->interval(interval);
  std::future<int> r = pool.commit(*p1);
  tm.AddTask("Pub1", std::move(p1));

  auto p2 = std::make_unique<rpcpp2::server::ReplyTask>(url + ".RepReq");
  p2->init(initRep, 33);
  p2->reply(reply);
  std::future<int> r2 = pool.commit(*p2);
  tm.AddTask("Rep1", std::move(p2));

//  pool.stop();
  pool.join_all();

  LOG_INFO << "initReply return: " << r.get();
  LOG_INFO << "initPublish return: " << r2.get();

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
