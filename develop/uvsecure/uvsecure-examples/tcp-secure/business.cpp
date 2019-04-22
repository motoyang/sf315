#include <string>
#include <iostream>
#include <sstream>
#include <memory>
#include <atomic>

#include <botan/hex.h>
#include <pp/prettyprint.h>

#include "secureacceptor.h"
#include "business.h"

// --

void Business::operator()() {
  std::srand(std::time(nullptr));
  while (_running) {
    std::vector<uvplus::Packet> packets;
    size_t count = _tcp->read(packets);
    for (int i = 0; i < count; ++i) {
      doSomething(packets.at(i));
    }
  }
  LOG_INFO << "business stopped.";

  _pool.stop();
  _pool.join_all();
  LOG_INFO << "thread pool stopped.";
}

void Business::doSomething(const uvplus::Packet &packet) {
  static size_t count = 0;
  static size_t bytes = 0;
  std::cout << "received " << packet._buf.size() << "B from: " << packet._peer << std::endl;
  std::cout << Botan::hex_encode(packet._buf) << std::endl;

  _tcp->write(packet._peer.c_str(), packet._buf.data(), packet._buf.size());
}

Business::Business(const std::string &name)
  :_name(name),
      _pool(1) {
}

void Business::stop() { _running.store(false); }

void Business::bind(SecureAcceptor *tcp) { _tcp = tcp; }
