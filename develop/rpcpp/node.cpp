#include <iostream>
#include <string>
#include <functional>
#include <chrono>
#include <thread>

#include <nanolog/nanolog.hpp>

#include <nng/nng.h>
#include <nng/protocol/pipeline0/pull.h>
#include <nng/protocol/pipeline0/push.h>

#include "rpcpp.h"
#include "node.h"
#include "node2.h"

// --

namespace rpcpp {

Node::Node(Socket::OpenFun f) : _sock(f) {}

bool Node::isRunning() const { return _sock.socketId() > 0; }

int Node::close() {
  return _sock.close();
}

int Node::dial(const char *url) { return _sock.dial(url); }

int Node::listen(const char *url) { return _sock.listen(url); }

int Node::recv(char **buf, size_t *len) { return _sock.recv(buf, len); }

int Node::send(void *data, size_t len) { return _sock.send(data, len); }

const char *Node::strerror(int e) { return nng_strerror(e); }

// --

Node2::Node2(const std::string& n, Socket::OpenFun f) : Object(n), _sock(f) {}

bool Node2::isRunning() const { return _sock.socketId() > 0; }

void Node2::close() {
 _sock.close();
}

int Node2::dial(const char *url) { return _sock.dial(url); }

int Node2::listen(const char *url) { return _sock.listen(url); }

int Node2::recv(char **buf, size_t *len) { return _sock.recv(buf, len); }

int Node2::send(void *data, size_t len) { return _sock.send(data, len); }

const char *Node2::strerror(int e) { return nng_strerror(e); }

} // namespace rpcpp
