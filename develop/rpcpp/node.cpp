#include <iostream>
#include <string>
#include <functional>
#include <chrono>
#include <thread>

#include <nanolog/nanolog.hpp>

#include <nng/nng.h>
#include <nng/protocol/pipeline0/pull.h>
#include <nng/protocol/pipeline0/push.h>

#include "threadpool.h"
#include "rpcpp.h"
#include "node.h"

// --

namespace rpcpp2 {

PushNode::PushNode(const std::string &url)
{
  LOG_INFO << "url: " << url;

  int rv = 0;
  if ((rv = nng_push0_open(&_sock)) != 0) {
    FATAL_EXIT(rv);
  }
  if ((rv = nng_dial(_sock, url.c_str(), NULL, 0)) != 0) {
    FATAL_EXIT(rv);
  }
}

int PushNode::send(void *buf, size_t len)
{
  int rv = 0;
  if ((rv = nng_send(_sock, buf, len, 0)) != 0) {
    if (rv != NNG_ECLOSED) {
      FATAL_EXIT(rv);
    }
 }
  return rv;
}

PullNode::PullNode(const std::string &url)
{
  LOG_INFO << "url: " << url;

  int rv = 0;
  if ((rv = nng_pull0_open(&_sock)) != 0) {
    FATAL_EXIT(rv);
  }
  if ((rv = nng_listen(_sock, url.c_str(), NULL, 0)) != 0) {
    FATAL_EXIT(rv);
  }
}

int PullNode::recv(char **buf, size_t *len)
{
  int rv= 0;
  if ((rv = nng_recv(_sock, buf, len, NNG_FLAG_ALLOC)) != 0) {
    if (rv != NNG_ECLOSED) {
      FATAL_EXIT(rv);
    }
  }
  return rv;
}

}




