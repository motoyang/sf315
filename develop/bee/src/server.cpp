#include <iostream>
#include <string>
#include <vector>
#include <utility>

#include "memoryimpl.h"
#include "packimpl.h"
#include "cryptography.h"
#include "recordlayer.h"
#include "cryptocenter.h"
#include "channel.h"
#include "server.h"

// --

Server::Server()
    : _rl(std::make_unique<RecordLayer>()),
      _channel(std::make_unique<Channel>()),
      _cryptocenter(std::make_unique<Cryptocenter>()) {
  _status.push(Server::Status::START);
}

void Server::run() const {
  for (auto v = _channel->recv(); v.size() > 0; v = _channel->recv()) {
    auto plaintext = (const TLSPlaintext *)v.data();
    auto fragment = plaintext->fragment();
    auto len = plaintext->length();
    if (plaintext->cryptoFlag()) {
      v = _cryptocenter->decrypto(fragment, len, _key);
      fragment = v.data();
      len = v.size();
    }
    auto l = _rl->feed(fragment, len);
    for (const auto &m : l) {
      std::cout << hex2section(secure::hex_encode(m.data(), m.size()), 4, 8)
                << std::endl;
      auto hs = (Handshake *)m.data();
      if (hs->msg_type() == HandshakeType::client_hello) {
        received((const ClientHello *)hs->message());
      }
    }
  }
}

void Server::received(const ClientHello *hello) const {

}