#include <string>
#include <functional>

#include <nanolog/nanolog.hpp>

#include "rpcpp.h"
#include "socket.h"

Socket::OpenFun OpenAsReq = nng_req_open;
Socket::OpenFun OpenAsRep = nng_rep_open;
Socket::OpenFun OpenAsPub = nng_pub_open;
Socket::OpenFun OpenAsSub = nng_sub_open;
Socket::OpenFun OpenAsPull = nng_pull_open;
Socket::OpenFun OpenAsPush = nng_push_open;
Socket::OpenFun OpenAsPair = nng_pair_open;
Socket::OpenFun OpenAsBus = nng_bus_open;
Socket::OpenFun OpenAsSurveyor = nng_surveyor_open;
Socket::OpenFun OpenAsRespondent = nng_respondent_open;

// --

Socket::Socket(OpenFun f) { f(&_sock); }

int Socket::close() {
  int r = nng_close(_sock);
  if (r) {
    FATAL_EXIT(r);
  }
  _sock = NNG_SOCKET_INITIALIZER;
  LOG_INFO << "socket closed: " << r;
  return r;
}

int Socket::dial(const char *url) {
  int r = nng_dial(_sock, url, 0, 0);
  if (r) {
    FATAL_EXIT(r);
  }
  return r;
}

int Socket::listen(const char *url) {
  int r = nng_listen(_sock, url, 0, 0);
  if (r) {
    FATAL_EXIT(r);
  }
  return r;
}

int Socket::recv(void *data, size_t *len) {
  int r = nng_recv(_sock, data, len, NNG_FLAG_ALLOC);
  if (r && (NNG_ETIMEDOUT != r) && (NNG_ECLOSED != r)) {
    FATAL_EXIT(r);
  }
  return r;
}

int Socket::send(void *data, size_t len) {
  int r = nng_send(_sock, data, len, 0);
  if (r) {
    FATAL_EXIT(r);
  }
  return r;
}
int Socket::setOpt(const char *opt, const void *val, size_t valsz) {
  int r = nng_setopt(_sock, opt, val, valsz);
  if (r) {
    FATAL_EXIT(r);
  }
  return r;
}

int Socket::getOpt(const char *opt, void *val, size_t *valszp) {
  int r = nng_getopt(_sock, opt, val, valszp);
  if (r) {
    FATAL_EXIT(r);
  }
  return r;
}

int Socket::socketId() const {
  int r = nng_socket_id(_sock);
  // if (r) {
  //   FATAL_EXIT(r);
  // }
  return r;
}