#include <iostream>
#include <string>
#include <functional>
#include <chrono>
#include <thread>

#include <nanolog/nanolog.hpp>

#include <nng/nng.h>
#include <nng/protocol/reqrep0/rep.h>
#include <nng/protocol/pubsub0/pub.h>

#include "threadpool.h"
#include "rpcpp.h"
#include "node.h"

namespace rpcpp2 {

  namespace server {

    void pubProcess(const char *url, std::function<std::string()>&& f) {
      using namespace std::chrono_literals;
      nng_socket sock;
      int rv= 0;

      if ((rv = nng_pub_open(&sock)) != 0) {
        FATAL_EXIT(rv);
      }
      if ((rv = nng_listen(sock, url, NULL, 0)) < 0) {
        FATAL_EXIT(rv);
      }
      while (gp_server->runable()) {
        std::string s = f();
        if ((rv = nng_send(sock, s.data(), s.size(), NNG_FLAG_NONBLOCK)) != 0) {
          if (rv == NNG_EAGAIN) {
            continue;
          } else {
            FATAL_EXIT(rv);
          }
        }
        std::this_thread::sleep_for(2s);
      }
      nng_close(sock);
    }

    void repProcess(const char *url, std::function<std::string(const char*, size_t len)>&& f) {
      nng_socket sock;
      int rv= 0;

      if ((rv = nng_rep0_open(&sock)) != 0) {
        FATAL_EXIT(rv);
      }
      if ((rv = nng_listen(sock, url, NULL, 0)) != 0) {
        FATAL_EXIT(rv);
      }

      while (gp_server->runable()) {
        char *buf = NULL;
        size_t sz;
        if ((rv = nng_recv(sock, &buf, &sz, NNG_FLAG_ALLOC | NNG_FLAG_NONBLOCK)) != 0) {
          if (rv == NNG_EAGAIN) {
            continue;
          } else {
            FATAL_EXIT(rv);
          }
        }
        std::string s = f(buf, sz);
        nng_send(sock, s.data(), s.size(), 0);
        nng_free(buf, sz);
      }

      nng_close(sock);
    }

  }
}
