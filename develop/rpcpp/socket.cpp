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
  _sock = NNG_SOCKET_INITIALIZER;
  LOG_INFO << "socket closed: " << r;
  return  r;
}

int Socket::dial(const char *url) { return nng_dial(_sock, url, 0, 0); }

int Socket::listen(const char *url) { return nng_listen(_sock, url, 0, 0); }

int Socket::recv(void *data, size_t *len) {
  return nng_recv(_sock, data, len, NNG_FLAG_ALLOC);
}

int Socket::send(void *data, size_t len) {
  return nng_send(_sock, data, len, 0);
}
int Socket::setOpt(const char *opt, const void *val, size_t valsz) {
  return nng_setopt(_sock, opt, val, valsz);
}
int Socket::getOpt(const char *opt, void *val, size_t *valszp) {
  return nng_getopt(_sock, opt, val, valszp);
}
int Socket::socketId() const { return nng_socket_id(_sock); }