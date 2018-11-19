#include <iostream>
#include <string>
#include <sstream>
#include <chrono>
#include <thread>
#include <atomic>

#include <nanolog/nanolog.hpp>

#include <msgpack-c/msgpack.hpp>

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
  LOG_TRACK;
  std::cout << i << ", " << s << std::endl;

  return 3;
}

int repFun2(int i, const std::string &s) {
  LOG_TRACK;
  std::cout << i << ", " << s << std::endl;

  return 6;
}

int quit() {
  LOG_TRACK;
  g_tmServer->stop();
  g_poolServer->stop();
  return 0;
}

std::string query() {
  LOG_TRACK;
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
//  r.show();

  return true;
}

// --

namespace rpcpp2 {

PushTask::PushTask(const std::string &url)
  : _node(url)
{
}

int PushTask::operator()()
{
  LOG_TRACK;
  while (_node.run()) {
    std::string s = _push();

    int rv = 0;
    if ((rv = _node.send(s.data(), s.size()))) {
      if (rv == NNG_ECLOSED) {
        break;
      }
    }
    _interval();
  }
  _node.close();

  LOG_INFO << "operator() will return with 0.";
  return 0;
}

void PushTask::close()
{
  _node.close();
}

// --

PullTask::PullTask(const std::string &url)
  : _node(url)
{
}

int PullTask::operator()()
{
  LOG_TRACK;
  if (!_init()) {
    return -1;
  }

  LOG_INFO << "_init() passed in operator().";
  int rv;
  while (_node.run()) {
    char *buf = NULL;
    size_t sz;
    if ((rv = _node.recv(&buf, &sz)) != 0) {
      if (rv == NNG_ECLOSED) {
        break;
      }
    }
    _pull(buf, sz);
    nng_free(buf, sz);
  }

  LOG_INFO << "operator() will return with 0.";
  return 0;
}

void PullTask::close()
{
  _node.close();
}

// --


// --

}

