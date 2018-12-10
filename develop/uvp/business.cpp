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

void Business::workCallback() {
  Packet packet;
  while (_running) {
    while (_gangway._upward.wait_dequeue_timed(packet, std::chrono::milliseconds(500))) {
      doSomething(std::move(packet));
    }
    // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    // LOG_INFO << "business is doing...";
  }
}

void Business::afterWorkCallback(int status) {
  if (status < 0) {
    LOG_IF_ERROR(status);
  }
  LOG_INFO << "afterwork called.";
}

void Business::doSomething(Packet &&packet) {
  char buf[512] = {0};
  memcpy(buf, packet._buf.base, packet._buf.len);
  std::cout << packet._peer << " -> " << buf << std::endl;

  BufT b = _codec.encode(packet._buf.base, packet._buf.len);
  _gangway._downward.enqueue(Packet(packet._peer, b));
  int r = _tcp->async()->send();
  LOG_IF_ERROR(r);
}

Business::Business(const std::string &name, Gangway &way)
    : _name(name), _gangway(way), _codec('|') {
  _work.workCallback(std::bind(&Business::workCallback, this));
  _work.afterWorkCallback(
      std::bind(&Business::afterWorkCallback, this, std::placeholders::_1));
}

int Business::start(LoopT *from) {
  int r = _work.queue(from);
  LOG_IF_ERROR(r);
  return r;
}

void Business::stop() { _running.store(false); }

void Business::tcp(TcpServer* tcp) {
  _tcp = tcp;
}