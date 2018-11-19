#include <iostream>
#include <string>
#include <map>
#include <sstream>
//#include <chrono>
//#include <thread>
//#include <atomic>

#include <nanolog/nanolog.hpp>

#include <msgpack.hpp>

#include <nng/nng.h>
#include <nng/protocol/reqrep0/rep.h>
#include <nng/protocol/pubsub0/pub.h>

#include "node.h"
#include "nodeclient.h"
#include "task.h"
#include "taskclient.h"

// --

namespace rpcpp2 {
namespace client {

// --

SubscribeTask::SubscribeTask(const std::string &url)
  : _node(url)
{
}

int SubscribeTask::operator()()
{
  if (!_init()) {
    return -1;
  }

  int rv;
  while (_node.run()) {
    char *buf = NULL;
    size_t sz;
    if ((rv = _node.recv(&buf, &sz)) != 0) {
      if (rv == NNG_ECLOSED) {
        break;
      }
    }
    _subscribe(buf, sz);
    nng_free(buf, sz);
  }

  return 0;
}

void SubscribeTask::close()
{
  _node.close();
}

// --

msgpack::object_handle RequestTask::sendAndRecv(const std::string &s)
{
  int rv = 0;
  if ((rv = _node.send((void*)s.data(), s.size())) != 0) {
    if (rv != NNG_ECLOSED) {
      return msgpack::object_handle();
    }
  }

  char *buf = NULL;
  size_t sz = 0;
  if ((rv = _node.recv(&buf, &sz)) != 0) {
    if (rv != NNG_ECLOSED) {
      return msgpack::object_handle();
    }
  }

  msgpack::object_handle oh = msgpack::unpack(buf, sz);
  nng_free(buf, sz);

  return (oh);
}

RequestTask::RequestTask(const std::string &url)
  : _node(url)
{
}

int RequestTask::operator()()
{
  int r = _request();
  return r;
}

void RequestTask::close()
{
  _node.close();
}

// --

}
}
