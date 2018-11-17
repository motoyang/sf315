#include <string>
#include <sstream>
#include <functional>

#include <nanolog/nanolog.hpp>

#include <msgpack.hpp>

#include <nng/nng.h>
#include <nng/protocol/reqrep0/req.h>
#include <nng/protocol/pubsub0/sub.h>

#include "threadpool.h"
#include "rpcpp.h"
#include "nodeClient.h"

namespace rpcpp2 {

  namespace client {

    NodeRequest::NodeRequest(const char* url) {
      int rv;
      if ((rv = nng_req0_open(&_sock)) != 0) {
        FATAL_EXIT(rv);
      }
      if ((rv = nng_dial(_sock, url, NULL, 0)) != 0) {
        FATAL_EXIT(rv);
      }
    }

    NodeRequest::~NodeRequest()
    {
      int rv;
      if ((rv = nng_close(_sock)) != 0) {
        FATAL_EXIT(rv);
      }
    }

    msgpack::object_handle NodeRequest::request(const std::string& s) const {
      int rv;
      size_t sz;
      char *buf = NULL;

      if ((rv = nng_send(_sock, (void*)s.data(), s.size(), 0)) != 0) {
        FATAL_EXIT(rv);
      }
      if ((rv = nng_recv(_sock, &buf, &sz, NNG_FLAG_ALLOC)) != 0) {
        FATAL_EXIT(rv);
      }

      msgpack::object_handle oh = msgpack::unpack(buf, sz);
      nng_free(buf, sz);

      return (oh);
    }

    // --

    NodeSubscriber::NodeSubscriber(const char *url)
    {
      int rv = 0;

      if ((rv = nng_sub0_open(&_sock)) != 0) {
        FATAL_EXIT(rv);
      }
      // subscribe to everything (empty means all topics)
      if ((rv = nng_setopt(_sock, NNG_OPT_SUB_SUBSCRIBE, "", 0)) != 0) {
        FATAL_EXIT(rv);
      }
      if ((rv = nng_dial(_sock, url, NULL, 0)) != 0) {
        FATAL_EXIT(rv);
      }
    }

    NodeSubscriber::~NodeSubscriber()
    {
    }

    void NodeSubscriber::close()
    {
      int rv;
      if ((rv = nng_close(_sock)) != 0) {
        FATAL_EXIT(rv);
      }
    }

    void NodeSubscriber::receive(std::function<void(msgpack::object_handle const&)>&& f) const
    {
      int rv;
      while (gp_client->runable()) {
        char *buf = NULL;
        size_t sz;
        //        if ((rv = nng_recv(_sock, &buf, &sz, NNG_FLAG_ALLOC | NNG_FLAG_NONBLOCK)) != 0) {
        if ((rv = nng_recv(_sock, &buf, &sz, NNG_FLAG_ALLOC)) != 0) {
          //          if (rv == NNG_EAGAIN) {
          //            continue;
          //          } else {
          if (rv == NNG_ECLOSED) {
            LOG_INFO << "socket subscribe closed.";
            break;
          }
          FATAL_EXIT(rv);
        }
        msgpack::object_handle oh = msgpack::unpack(buf, sz);
        f(oh);
        nng_free(buf, sz);
      }
    }
  }
}


