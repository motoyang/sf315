#include <string>
#include <iostream>
#include <sstream>
#include <memory>
#include <atomic>

#include <pp/prettyprint.h>

#include "business.h"

// --

void Business::operator()() {
  std::srand(std::time(nullptr));
  while (_running) {
    std::vector<uvplus::Packet> packets;
    size_t count = _tcp->upwardDequeue(packets);
    for (int i = 0; i < count; ++i) {
      doSomething(packets.at(i));
    }
  }
  LOG_INFO << "business stopped.";

  _pool.stop();
  _pool.join_all();
  LOG_INFO << "thread pool stopped.";
}

int f31(int i, int j) {
  int r = i + j;
  std::cout << i << " + " << j << " = " << r << std::endl;
  return r;
}

auto f32() {
  int i = std::rand() % 100, j = std::rand() % 200;
  std::string s("abcde");
  auto r = std::make_tuple(i, s, j);
  std::cout << r << std::endl;
  return r;
}

void f23(int i, double d, const std::string &s) {
  static size_t count = 0;
  if (++count % 10000) return;
  std::cout << "f23: i = " << i << ", d = " << d << ", s = " << s << std::endl;
}

void Business::doSomething(const uvplus::Packet &packet) {
  static size_t count = 0;
  static size_t bytes = 0;

  bytes += packet._buf.len;
  if ((++count % 10000) == 0) {
    std::cout << "received: " << count << " packets, " << bytes << " bytes."
              << std::endl;
  }

  uvp::uv::BufT buf = packet._buf;
  size_t offset = 0;
  int buf_type = 0;
  msgpack::object_handle oh = msgpack::unpack(buf.base, buf.len, offset);
  oh.get().convert(buf_type);

  if (buf_type == (int)uvp::BufType::BUF_REQ_TYPE) {
    msgpack::object_handle oh2 = msgpack::unpack(buf.base, buf.len, offset);
    int token = 0;
    oh2.get().convert(token);

    std::string result = _replier->reply(buf.base, buf.len, offset);

    std::stringstream ss;
    msgpack::pack(ss, (int)uvp::BufType::BUF_REP_TYPE);
    msgpack::pack(ss, token);
    msgpack::pack(ss, result);
    int r = _tcp->downwardEnqueue(packet._peer.c_str(), ss.str().data(),
                                   ss.str().length());
    UVP_LOG_ERROR(r);
    return;
  }

  if (buf_type == (int)uvp::BufType::BUF_ECHO_TYPE) {
    int r = _tcp->downwardEnqueue(packet._peer.c_str(), packet._buf.base,
                                   packet._buf.len);
    UVP_LOG_ERROR(r);
    return;
  }

  if (buf_type == (int)uvp::BufType::BUF_RESOLVE_TYPE) {
    msgpack::object_handle oh2 = msgpack::unpack(buf.base, buf.len, offset);
    _resolver->resolve(buf.base, buf.len, offset);
  }
}

Business::Business(const std::string &name)
    : _replier(std::make_unique<uvplus::Replier<int>>()),
      _resolver(std::make_unique<uvplus::Resolver<int>>()), _name(name),
      _pool(1) {
  _replier->defineFun(31, f31).defineFun(32, f32);
  _replier->show(std::cout);

  _resolver->defineFun(23, f23);
  _resolver->show(std::cout);
}

void Business::stop() { _running.store(false); }

void Business::bind(uvplus::TcpAcceptor *tcp) { _tcp = tcp; }
