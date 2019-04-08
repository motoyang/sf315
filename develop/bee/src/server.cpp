#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>

#include "memoryimpl.h"
#include "packimpl.h"
#include "cryptography.h"
#include "recordlayer.h"
#include "cryptocenter.h"
#include "versionsupported.h"
#include "channel.h"
#include "server.h"

// --

Server::Server()
    : _vs(std::make_unique<VersionSupported>()),
      _rl(std::make_unique<RecordLayer>()),
      _channel(std::make_unique<Channel>()),
      _cryptocenter(std::make_unique<Cryptocenter>()) {
  _status.push(Server::Status::START);
}

void Server::run() {
  for (auto v = _channel->recv(); v.size() > 0; v = _channel->recv()) {
    // v可能是TSLPlaintext或者TSLCiphertext，头都是相同的。
    auto plaintext = (const TLSPlaintext *)v.data();
    auto fragment = plaintext->fragment();
    auto len = plaintext->length();
    auto ct = plaintext->type;
    if (plaintext->cryptoFlag()) {
      // v是TSLCiphertext，要解密后处理
      v = _cryptocenter->decrypto(fragment, len, _key);
      auto ct_addr =
          std::find_if(v.crbegin(), v.crend(), [](uint8_t c) { return c > 0; });
      if (ct_addr == v.crbegin()) {
        assert(false);
        continue;
      }
      ct = ContentType(*ct_addr);
      auto distance = std::distance(ct_addr, v.crbegin());
      v.resize(distance - 1); // -1是为了排除TLSInnerText.type
      fragment = v.data();
      len = v.size();
    }
    auto l = _rl->feed(fragment, len);
    assert(l.size() <= 1);
    for (const auto &m : l) {
      std::cout << hex2section(secure::hex_encode(m.data(), m.size()), 4, 8)
                << std::endl;
      received(ct, m.data(), m.size());
    }
  }
}

void Server::received(const ClientHello *hello) {
  _status.pop();
  _status.push(Server::Status::RECVD_CH);
}

void Server::received(ContentType ct, const uint8_t *p, size_t len) {
  switch (ct) {
  case ContentType::handshake: {
    while (len > 0) {
      auto hs = (Handshake *)p;
      len -= hs->size();
      p += len;

      switch (hs->msg_type()) {
      case HandshakeType::client_hello: {
        auto ch = (ClientHello *)hs->message();
        std::vector<uint8_t> buf(4096);
        hs = hello(buf, ch);
        std::cout << hex2section(secure::hex_encode((uint8_t *)hs, hs->size()),
                                 4, 8)
                  << std::endl;
      } break;

      default:
        break;
      }
    }
  }
  // hearHandshake();
  break;
  case ContentType::application_data:
    break;
  case ContentType::alert:
    break;
  case ContentType::change_cipher_spec:
  case ContentType::invalid:
  default:
    assert(false);
    break;
  }
}

Handshake *Server::hello(std::vector<uint8_t> &buf, const ClientHello *ch) {
  CipherSuite cs;
  if (!_cryptocenter->select(cs, ch->cipher_suites())) {
    assert(false);
  }

  std::unique_ptr<secure::RandomNumberGenerator> rng(
      new secure::AutoSeeded_RNG);

  auto hs = (Handshake *)buf.data();
  hs->msg_type(HandshakeType::server_hello);

  auto sh = (ServerHello *)hs->message();
  sh->legacy_version = PV_TLS_1_2;
  rng->randomize(sh->random, sizeof(sh->random));
  auto lsie = sh->legacy_session_id_echo();
  lsie->len(ch->legacy_session_id()->len());
  for (auto i = 0; i < lsie->len(); ++i) {
    lsie->data()[i] = ch->legacy_session_id()->data()[i];
  }
  sh->cipher_suite()[0] = cs;
  sh->legacy_compression_method()[0] = 0;

  auto ex1 = (Extension<ServerSupportedVersion> *)sh->extensions()->data();
  ex1->extension_type = ExtensionType::supported_versions;
  ex1->extension_data()[0] = _vs->selected();

  auto ex2 = (Extension<KeyShareServerHello> *)ex1->next();
  ex2->extension_type = ExtensionType::key_share;
  auto kse = ex2->extension_data();
  kse->group = NamedGroup::ffdhe2048;
  auto pk = _cryptocenter->privateKey(kse->group)->public_value();
  kse->key_exchange()->len(pk.size());
  auto key = kse->key_exchange()->data();
  MemoryInterface::get()->copy(key, pk.data(), pk.size());

  sh->extensions()->len(ex1->size() + ex2->size());
  hs->length(sh->size());

  assert(buf.size() > hs->size());
  buf.resize(hs->size());
  return (Handshake *)buf.data();
}