#include <string>
#include <sstream>
#include <functional>

#include <nanolog/nanolog.hpp>

#include <msgpack-c/msgpack.hpp>

#include <nng/nng.h>
#include <nng/protocol/reqrep0/req.h>
#include <nng/protocol/pubsub0/sub.h>
#include <nng/protocol/pair0/pair.h>

#include "threadpool.h"
#include "rpcpp.h"
#include "node.h"
#include "nodeclient.h"

namespace rpcpp2 {
namespace client {

// --

SubscribeNode::SubscribeNode(const std::string &url)
{
  LOG_INFO << "url: " << url;

  int rv = 0;
  if ((rv = nng_sub_open(&_sock)) != 0) {
    FATAL_EXIT(rv);
  }
  // subscribe to everything (empty means all topics)
  if ((rv = nng_setopt(_sock, NNG_OPT_SUB_SUBSCRIBE, "", 0)) != 0) {
    FATAL_EXIT(rv);
  }
  if ((rv = nng_dial(_sock, url.c_str(), NULL, 0)) != 0) {
    FATAL_EXIT(rv);
  }
}

int SubscribeNode::recv(char **buf, size_t *len)
{
  int rv = 0;
  if ((rv = nng_recv(_sock, buf, len, NNG_FLAG_ALLOC)) != 0) {
    if (rv != NNG_ECLOSED) {
      FATAL_EXIT(rv);
    }
  }
  return rv;
}

// --

RequestNode::RequestNode(const std::string &url)
{
  LOG_INFO << "url: " << url;

  int rv = 0;
  if ((rv = nng_req_open(&_sock)) != 0) {
    FATAL_EXIT(rv);
  }
  if ((rv = nng_dial(_sock, url.c_str(), NULL, 0)) != 0) {
    FATAL_EXIT(rv);
}

}

int RequestNode::send(void *buf, size_t len)
{
  int rv = 0;
  if ((rv = nng_send(_sock, buf, len, 0)) != 0) {
    if (rv != NNG_ECLOSED) {
      FATAL_EXIT(rv);
    }
  }
  return rv;
}

int RequestNode::recv(char **buf, size_t *len)
{
  int rv = 0;
  if ((rv = nng_recv(_sock, buf, len, NNG_FLAG_ALLOC)) != 0) {
    if (rv != NNG_ECLOSED) {
      FATAL_EXIT(rv);
    }
  }
  return rv;
}

// --


PairNode::PairNode(const std::string &url)
{
  LOG_INFO << "url: " << url;

  int rv = 0;
  if ((rv = nng_pair_open(&_sock)) != 0) {
    FATAL_EXIT(rv);
  }
   if ((rv = nng_dial(_sock, url.c_str(), NULL, 0)) !=0) {
     FATAL_EXIT(rv);
   }
   if ((rv = nng_setopt_ms(_sock, NNG_OPT_RECVTIMEO, 100)) != 0) {
    FATAL_EXIT(rv);
    }
}

int PairNode::recv(char **buf, size_t *len)
{
  int rv= 0;
  if ((rv = nng_recv(_sock, buf, len, NNG_FLAG_ALLOC)) != 0) {
    if ((rv != NNG_ECLOSED) && (rv != NNG_ETIMEDOUT)) {
      FATAL_EXIT(rv);
    }
  }
  return rv;
}

int PairNode::send(void *buf, size_t len)
{
  int rv= 0;
  if ((rv = nng_send(_sock, buf, len, 0)) != 0) {
    if (rv != NNG_ECLOSED) {
      FATAL_EXIT(rv);
    }
  }
  return rv;
}

// --

}
}

