#include <iostream>
//#include <string>
//#include <functional>
//#include <chrono>
//#include <thread>

#include <nanolog/nanolog.hpp>

#include <nng/nng.h>
#include <nng/protocol/reqrep0/rep.h>
#include <nng/protocol/pubsub0/pub.h>
#include <nng/protocol/pair0/pair.h>

#include "rpcpp.h"
#include "node.h"
#include "nodeserver.h"


namespace rpcpp2 {
namespace server {

// --

PublishNode::PublishNode(const std::string &url)
{
  LOG_INFO << "url: " << url;

  int rv= 0;
  if ((rv = nng_pub_open(&_sock)) != 0) {
    FATAL_EXIT(rv);
  }
  if ((rv = nng_listen(_sock, url.c_str(), NULL, 0)) < 0) {
    FATAL_EXIT(rv);
  }
}

int PublishNode::send(void *buf, size_t len)
{
  int rv = 0;
  if ((rv = nng_send(_sock, buf, len, 0)) != 0) {
    if (rv != NNG_ECLOSED) {
      FATAL_EXIT(rv);
    }
 }
  return rv;
}

// --

ReplyNode::ReplyNode(const std::string &url)
{
  LOG_INFO << "url: " << url;

  int rv= 0;
  if ((rv = nng_rep_open(&_sock)) != 0) {
    FATAL_EXIT(rv);
  }
  if ((rv = nng_listen(_sock, url.c_str(), NULL, 0)) != 0) {
    FATAL_EXIT(rv);
  }
}

int ReplyNode::recv(char **buf, size_t *len)
{
  int rv= 0;
  if ((rv = nng_recv(_sock, buf, len, NNG_FLAG_ALLOC)) != 0) {
    if (rv != NNG_ECLOSED) {
      FATAL_EXIT(rv);
    }
  }
  return rv;
}

int ReplyNode::send(void *buf, size_t len)
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

PairNode::PairNode(const std::string &url)
{
  LOG_INFO << "url: " << url;

  int rv = 0;
  if ((rv = nng_pair_open(&_sock)) != 0) {
    FATAL_EXIT(rv);
  }
  if ((rv = nng_listen(_sock, url.c_str(), NULL, 0)) !=0) {
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
