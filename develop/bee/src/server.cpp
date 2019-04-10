#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>

#include "nanolog.hpp"
#include "prettyprint.h"
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
    auto bRun = true;

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
      bRun = false;
    }
    // TBF!!!
    if (!bRun) break;
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
        auto sh = hello(buf, ch);
        sayHello(sh);
        _cryptocenter->hashUpdate((uint8_t*) ch, ch->size());
          _cryptocenter->hashUpdate((uint8_t*) sh, sh->size());

        _cryptocenter->driveKey(ch, (ServerHello*)sh->message());
        std::cout << "end of sayHello from server" << std::endl;
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
  // 解析Extensions到map，以便后续使用
  auto exMap = extensionsCheck(ch->extensions());

  // 如果legacy_version不是PV_TLS_1_2(0x0303)，或者没有supported_versions扩展。
  if (ch->legacy_version != PV_TLS_1_2 ||
      !exMap.find(ExtensionType::supported_versions)->second) {
    // 中断握手过程。TLS1.3规定，legacy_version必须是PV_TLS_1_2(0x0303)，版本协商通过
    // SupportedVersions扩张完成。
    // from rfc8446: Servers MAY abort the handshake upon receiving a
    // ClientHello with legacy_version 0x0304 or later.

    // 发送protocol_version Alert？
    return nullptr;
  }

  // 开始Handshake
  auto hs = (Handshake *)buf.data();
  hs->msg_type(HandshakeType::server_hello);

  // 开始ServerHello
  auto sh = (ServerHello *)hs->message();
  sh->legacy_version = PV_TLS_1_2;
  _cryptocenter->rng()->randomize(sh->random, sizeof(sh->random));

  // 原样返回ClientHello.legacy_session
  // from RFC8446: The contents of the client’s legacy_session_id field.
  auto lsie = sh->legacy_session_id_echo();
  lsie->len(ch->legacy_session_id()->len());
  for (auto i = 0; i < lsie->len(); ++i) {
    lsie->data()[i] = ch->legacy_session_id()->data()[i];
  }

  if (!_cryptocenter->select(sh->cipher_suite(), ch->cipher_suites())) {
    // 没有匹配到CipherSuit，填充Server希望的CipherSuit。Client在收到后，发现Server的
    // CipherSuit不在自己发送的CipherSuitsList中，会中断握手过程。
    // from RFC8446: A client which receives a cipher suite that was not offered
    // MUST abort the handshake with an "illegal_parameter" alert.
    NLOG_WARN << "Server can't found matched cipher suit from ClientHello, so "
                 "returened cipher suit is "
              << sh->cipher_suite()[0] << ".";
  }
  sh->legacy_compression_method()[0] = 0;

  auto ex1 = (Extension<ServerSupportedVersion> *)sh->extensions()->data();
  ex1->extension_type = ExtensionType::supported_versions;

  if (!_vs->selected(ex1->extension_data(),
                     (ClientSupportedVersions *)exMap.at(
                         ExtensionType::supported_versions))) {
    // If the "supported_versions" extension in the ServerHello contains a
    // version not offered by the client or contains a version prior to
    // TLS 1.3, the client MUST abort the handshake with an "illegal_parameter"
    // alert.
    NLOG_WARN << "Server can't found TLS1.3 version in ClientHello, so "
                 "returned version is "
              << *ex1->extension_data() << ".";
  }

  auto ex2 = (Extension<KeyShareServerHello> *)ex1->next();
  ex2->extension_type = ExtensionType::key_share;
  auto kse = ex2->extension_data();
  if (!_cryptocenter->select(
          kse, (KeyShareClientHello *)exMap.at(ExtensionType::key_share))) {
    NLOG_WARN << "Server can't found matched KeyShareEntry in ClientHello, so "
                 "HelloRetryRequest will been sent.";
    helloRetryRequest(sh);

    NLOG_WARN << "Server can't found matched KeyShareEntry.";
  }

  sh->extensions()->len(ex1->size() + ex2->size());
  hs->length(sh->size());

  assert(buf.size() > hs->size());
  buf.resize(hs->size());
  return (Handshake *)buf.data();
}

std::unordered_map<ExtensionType, uint8_t *>
Server::extensionsCheck(Extensions *e) {
  std::unordered_map<ExtensionType, uint8_t *> exMap;
  auto len = e->len();
  auto ex = e->data();
  while (len > 0) {
    switch (ex->extension_type) {
    case ExtensionType::supported_versions: {
      exMap.insert({ex->extension_type, ex->extension_data()});
      auto ee = (Extension<ClientSupportedVersions> *)ex;
      ex = (decltype(ex))ee->next();
      len -= ee->size();
    } break;
    case ExtensionType::supported_groups: {
      exMap.insert({ex->extension_type, ex->extension_data()});
      auto ee = (Extension<NamedGroupList> *)ex;
      ex = (decltype(ex))ee->next();
      len -= ee->size();
    } break;
    case ExtensionType::key_share: {
      exMap.insert({ex->extension_type, ex->extension_data()});
      auto ee = (Extension<KeyShareClientHello> *)ex;
      ex = (decltype(ex))ee->next();
      len -= ee->size();
    } break;
    default:
      break;
    }
  }

  return exMap;
}

void Server::sayHello(const Handshake *hs) {
  std::cout << hex2section(secure::hex_encode((uint8_t *)hs, hs->size()), 4, 8)
            << std::endl;

  auto pl = _rl->fragment(ContentType::handshake, (uint8_t *)hs, hs->size());
  for (const auto &p : pl) {
    _channel->send((uint8_t *)p, p->size());
  }

  _status.pop();
  _status.push(Server::Status::NEGOTIATED);
}

void Server::helloRetryRequest(ServerHello *sh) const {
  constexpr uint8_t retryRandom[] = {
      0xCF, 0x21, 0xAD, 0x74, 0xE5, 0x9A, 0x61, 0x11, 0xBE, 0x1D, 0x8C,
      0x02, 0x1E, 0x65, 0xB8, 0x91, 0xC2, 0xA2, 0x11, 0x16, 0x7A, 0xBB,
      0x8C, 0x5E, 0x07, 0x9E, 0x09, 0xE2, 0xC8, 0xA8, 0x33, 0x9C};
  MemoryInterface::get()->copy(sh->random, retryRandom, sizeof(retryRandom));
}