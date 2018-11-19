#include <iostream>
#include <string>
#include <sstream>
#include <chrono>
#include <thread>
#include <atomic>

#include <nanolog/nanolog.hpp>

#include <msgpack.hpp>

#include <nng/nng.h>
#include <nng/protocol/reqrep0/rep.h>
#include <nng/protocol/pubsub0/pub.h>

#include "rpcpp.h"
#include "register.h"
#include "threadpool.h"
#include "node.h"
#include "nodeserver.h"
#include "task.h"

extern rpcpp2::threadpool* g_poolServer;
extern rpcpp2::TaskManager* g_tmServer;

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

  g_tmServer->stop();
  g_poolServer->stop();
  return 0;
}

std::string query() {
  std::cout << __FUNCTION__ << " was called." << std::endl;
  return rpcpp2::server::Register::instance().query();
}

// --

class AddObject {
public:
  int add(int i, int j) { return (i + j) * 2; }
  int sub(int i, int j) { return (i - j) * 2; }
};

bool initRep(int i) {
  ++i;
  rpcpp2::server::Register &r = rpcpp2::server::Register::instance();

  r.exportFun("repFun1", repFun1).exportFun("repFun2", repFun2)
      .exportFun("quit", quit).exportFun("query", query);
  r.defineClass(std::string(rpcpp2::server::demangle(typeid(AddObject).name())))
      .exportMethod("add", &AddObject::add)
      .exportMethod("sub", &AddObject::sub);
  r.show();

  return true;
}

// --

