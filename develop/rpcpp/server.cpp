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

#include "anyarg.h"
#include "threadpool.h"
#include "node.h"
#include "Register.h"
#include "server.h"

// --

std::threadpool* gp_server = nullptr;

// --

int repFun1(int i, const std::string &s) {
  std::cout << __FUNCTION__ << " was called." << std::endl;
  std::cout << i << ", " << s << std::endl;

  return 3;
}

int repFun2(int i, const std::string &s) {
  std::cout << __FUNCTION__ << " was called." << std::endl;
  std::cout << i << ", " << s << std::endl;

  return 6;
}

int quit() {
  std::cout << __FUNCTION__ << " was called." << std::endl;
  gp_server->stop();
  return 0;
}

std::string query() {
  std::cout << __FUNCTION__ << " was called." << std::endl;
  return rpcpp2::server::Register::instance().query();
}

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

class AddObject {
public:
  int add(int i, int j) { return (i + j) * 2; }
  int sub(int i, int j) { return (i - j) * 2; }
};

// --

int initReply(const char* url) {
  rpcpp2::server::Register &r = rpcpp2::server::Register::instance();

  r.exportFun("repFun1", repFun1).exportFun("repFun2", repFun2)
      .exportFun("quit", quit).exportFun("query", query);
  r.defineClass(std::string(rpcpp2::server::demangle(typeid(AddObject).name())))
      .exportMethod("add", &AddObject::add)
      .exportMethod("sub", &AddObject::sub);
  r.show();

  std::string withSuffix(url);
  withSuffix += ".RepReq";

  LOG_INFO << "repProcess started...";
  rpcpp2::server::repProcess(withSuffix.c_str(), reply);
  LOG_INFO << "repProcess end.";

  return 0;
}

int initPublish(const char* url) {
  auto f = []() {
    std::time_t t = std::time(nullptr);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&t), "%F %T");
    std::stringstream ss2;
    msgpack::pack(ss2, ss.str());

    return ss2.str();
  };

  std::string withSuffix(url);
  withSuffix += ".PubSub";

  LOG_INFO << "pubProcess start.";
  rpcpp2::server::pubProcess(withSuffix.c_str(), f);
  LOG_INFO << "pubProcess end.";

  return 0;
}

// --

int startServer(const Anyarg& opt) {
  std::string url = opt.get_value_str('u');

  // 服务器的线程池
  std::threadpool pool(4);
  gp_server = &pool;

  std::future<int> r = gp_server->commit(initReply, url.c_str());
  std::future<int> r2 = gp_server->commit(initPublish, url.c_str());

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
