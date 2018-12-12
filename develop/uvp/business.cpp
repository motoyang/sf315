#include <string>
#include <iostream>
#include <sstream>
#include <atomic>

// #include <uv.h>
// #include <types.hpp>
// #include <ringbuffer.hpp>
// #include <misc.hpp>
#include <utilites.hpp>
#include <uv.hpp>
#include <req.hpp>
// #include <gangway.hpp>

#include "server.h"
#include "business.h"

// --

void Business::operator()() {
  while (_running) {
    Packet packet;
    // if (_tcp->gangway()._upward.wait_dequeue_timed(
    //         packet, std::chrono::milliseconds(500))) {
    if (_tcp->upwardDequeue(packet)) {
      doSomething(packet);
    }
  }
}

void Business::doSomething(const Packet &packet) {
  static size_t count = 0;
  if ((++count % 10000) == 0) {
    std::cout << "received: " << count << " packets." << std::endl;    
  }

  // char buf[512] = {0};
  // memcpy(buf, packet._buf.base, packet._buf.len);
  // std::cout << packet._peer << " -> " << buf << std::endl;

  // BufT b = _codec.encode(packet._buf.base, packet._buf.len);
  // _tcp->gangway()._downward.enqueue(Packet(packet._peer, b));
  // int r = _tcp->async()->send();
  // int r = _tcp->downwardEnqueue(Packet(packet._peer, b));
  int r = _tcp->downwardEnqueue(packet._peer.c_str(), packet._buf.base,
                                packet._buf.len);
  LOG_IF_ERROR(r);
}

Business::Business(const std::string &name) : _name(name) {}

void Business::stop() { _running.store(false); }

void Business::bind(TcpAcceptor *tcp) { _tcp = tcp; }