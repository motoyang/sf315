#include <string>
#include <iostream>
#include <sstream>
#include <memory>
#include <atomic>

#include <utilites.hpp>
#include <uv.hpp>
#include <req.hpp>

#include <resolver.h>
#include "server.h"
#include "business.h"

// --

void Business::operator()() {
  while (_running) {
    Packet packet;
    if (_tcp->upwardDequeue(packet)) {
      doSomething(packet);
    }
  }
  LOG_INFO << "business stoped.";
}

int f31(int i, int j) {
  int r = i + j;
  std::cout << i << " + " << j << " = " << r << std::endl;
  return r;
}

void f23(int i, double d, const std::string &s) {
  std::cout << "f23: i = " << i << ", d = " << d << ", s = " << s << std::endl;
}

void Business::doSomething(const Packet &packet) {
  static size_t count = 0;
  static size_t bytes = 0;

  bytes += packet._buf.len;
  if ((++count % 1000) == 0) {
    std::cout << "received: " << count << " packets, " << bytes << " bytes."
              << std::endl;
  }

  BufT buf = packet._buf;
  size_t offset = 0;
  int buf_type = 0;
  msgpack::object_handle oh = msgpack::unpack(buf.base, buf.len, offset);
  oh.get().convert(buf_type);
  if (buf_type == (int)BufType::BUF_REQ_TYPE) {
    msgpack::object_handle oh2 = msgpack::unpack(buf.base, buf.len, offset);
    int token = 0;
    oh2.get().convert(token);

    std::string result = _replier->reply(buf.base, buf.len, offset);

    std::stringstream ss;
    msgpack::pack(ss, (int)BufType::BUF_REP_TYPE);
    msgpack::pack(ss, token);
    msgpack::pack(ss, result);
    int r = _tcp->downwardEnqueue(packet._peer.c_str(), ss.str().data(),
                                   ss.str().length());
    LOG_IF_ERROR(r);
    return;
  }

  if (buf_type == (int)BufType::BUF_ECHO_TYPE) {
    int r = _tcp->downwardEnqueue(packet._peer.c_str(), packet._buf.base,
                                   packet._buf.len);
    LOG_IF_ERROR(r);
    return;
  }

  if (buf_type == (int)BufType::BUF_RESOLVE_TYPE) {
    msgpack::object_handle oh2 = msgpack::unpack(buf.base, buf.len, offset);
    _resolver->resolve(buf.base, buf.len, offset);
  }
}

Business::Business(const std::string &name)
    : _replier(std::make_unique<uvp::Replier<int>>()),
      _resolver(std::make_unique<uvp::Resolver<int>>()), _name(name) {
  _replier->defineFun(31, f31);
  _resolver->defineFun(23, f23);
}

void Business::stop() { _running.store(false); }

void Business::bind(TcpAcceptor *tcp) { _tcp = tcp; }
