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
#include "taskserver.h"

// --

namespace rpcpp2 {
namespace server {

// --

PublishTask::PublishTask(const std::string &url)
: _node(url)
{
}

int PublishTask::operator() ()
{
  LOG_TRACK;

  while (_node.run()) {
    std::string s = _publish();

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

void PublishTask::close()
{
  _node.close();
}

// --

ReplyTask::ReplyTask(const std::string &url)
  : _node(url)
{
}

int ReplyTask::operator()()
{
  LOG_TRACK;
  if (!_init()) {
    return -1;
  }
  LOG_INFO << "_init() passed in operator().";

  while (_node.run()) {
    char *buf = NULL;
    size_t sz = 0;
    int rv = 0;
    if ((rv = _node.recv(&buf, &sz)) != 0) {
      if (rv == NNG_ECLOSED) {
        break;
      }
    }
    std::string s = _reply(buf, sz);
    nng_free(buf, sz);

    if ((rv = _node.send(s.data(), s.size())) != 0) {
      if (rv == NNG_ECLOSED) {
        break;
      }
    }
  }

  LOG_INFO << "operator() will return with 0.";
  return 0;
}

void ReplyTask::close()
{
  _node.close();
}

// --

}
}
