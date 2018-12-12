#include <string>
#include <iostream>
#include <sstream>
#include <atomic>

#include <utilites.hpp>
#include <uv.hpp>
#include <req.hpp>

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

void Business::doSomething(const Packet &packet) {
  static size_t count = 0;
  static size_t bytes = 0;

  bytes += packet._buf.len;
  if ((++count % 10000) == 0) {
    std::cout << "received: " << count << " packets, " << bytes << " bytes." 
        << std::endl;    
  }

  int r = _tcp->downwardEnqueue(packet._peer.c_str(), packet._buf.base,
                                packet._buf.len);
  LOG_IF_ERROR(r);
}

Business::Business(const std::string &name) : _name(name) {}

void Business::stop() { _running.store(false); }

void Business::bind(TcpAcceptor *tcp) { _tcp = tcp; }
