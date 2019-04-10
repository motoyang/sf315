#include <iostream>
#include <string>
#include <vector>
#include <utility>

#include "memoryimpl.h"
#include "packimpl.h"
#include "cryptography.h"
#include "recordlayer.h"
#include "cryptocenter.h"
#include "versionsupported.h"
#include "channel.h"
#include "client.h"

// --

// --

Client::Client()
    : _vs(std::make_unique<VersionSupported>()),
      _rl(std::make_unique<RecordLayer>()),
      _channel(std::make_unique<Channel>()),
      _cryptocenter(std::make_unique<Cryptocenter>()) {
  _status.push(Client::Status::START);
}

void Client::start() {
  std::vector<uint8_t> buf(12024, 0);
  sayHello(hello(buf));
  _clientHello = buf;
}

const Handshake *Client::hello(std::vector<uint8_t> &buf) const {
  std::unique_ptr<secure::RandomNumberGenerator> rng(
      new secure::AutoSeeded_RNG);

  auto hs_frame = (Handshake *)buf.data();
  hs_frame->msg_type(HandshakeType::client_hello);
  // hs_frame->length(259);

  auto ch = (ClientHello *)hs_frame->message();
  ch->legacy_version = PV_TLS_1_2;
  rng->randomize(ch->random, sizeof(ch->random));
  ch->legacy_session_id()->len(0);
  _cryptocenter->support(ch->cipher_suites());
  ch->legacy_compression_methods()->len(1);
  ch->legacy_compression_methods()->data()[0] = 0;

  auto ex1 = (Extension<ClientSupportedVersions> *)ch->extensions()->data();
  ex1->extension_type = ExtensionType::supported_versions;
  _vs->supported(ex1->extension_data());

  auto ex2 = (Extension<NamedGroupList> *)ex1->next();
  ex2->extension_type = ExtensionType::supported_groups;
  _cryptocenter->support(ex2->extension_data());

  auto ex3 = (Extension<KeyShareClientHello> *)ex2->next();
  ex3->extension_type = ExtensionType::key_share;
  auto ksch = ex3->extension_data();
  _cryptocenter->support(ksch);

  ch->extensions()->len(ex1->size() + ex2->size() + ex3->size());
  hs_frame->length(ch->size());

  buf.resize(hs_frame->size());
  return (const Handshake *)buf.data();
}

void Client::sayHello(const Handshake *hs) {
  std::cout << hex2section(secure::hex_encode((uint8_t *)hs, hs->size()), 4, 8)
            << std::endl;

  auto pl = _rl->fragment(ContentType::handshake, (uint8_t *)hs, hs->size());
  for (const auto &p : pl) {
    _channel->send((uint8_t *)p, p->size());
  }

  _status.pop();
  _status.push(Client::Status::WAIT_SH);
}

void Client::sayAppdata(const uint8_t *p, size_t len) {
  // TBF!!!
  auto pv = _rl->fragmentWithPadding(ContentType::application_data, p, len);
  for (const auto &v : pv) {
    auto cv = _cryptocenter->crypto(v, _key);
    _channel->send(cv.data(), cv.size());
  }
}

void Client::run() {
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

void Client::received(ContentType ct, const uint8_t *p, size_t len) {
  switch (ct) {
  case ContentType::handshake: {
    while (len > 0) {
      auto hs = (Handshake *)p;
      len -= hs->size();
      p += len;

      switch (hs->msg_type()) {
      case HandshakeType::server_hello : {
        auto sh = (ServerHello *)hs->message();
        received(sh);
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

void Client::received(const ServerHello* sh) {

  auto exMap = extensionsCheck(sh->extensions());
  _cryptocenter->cipherSuitInit(*sh->cipher_suite());

  _cryptocenter->hashUpdate(_clientHello.data(), _clientHello.size());
  _cryptocenter->hashUpdate((uint8_t*)sh, sh->size());

  auto kse = (KeyShareEntry*)exMap.at(ExtensionType::key_share);
  auto len = kse->key_exchange()->len();
  auto data = kse->key_exchange()->data();
  std::vector<uint8_t> publicKey(data, data + len);
  auto b = _cryptocenter->driveKey(kse->group, publicKey);

  _cryptocenter->driveKey(nullptr, nullptr);
}

std::unordered_map<ExtensionType, uint8_t *>
Client::extensionsCheck(Extensions *e) {
  std::unordered_map<ExtensionType, uint8_t *> exMap;
  auto len = e->len();
  auto ex = e->data();
  while (len > 0) {
    switch (ex->extension_type) {
    case ExtensionType::supported_versions: {
      exMap.insert({ex->extension_type, ex->extension_data()});
      auto ee = (Extension<ServerSupportedVersion> *)ex;
      ex = (decltype(ex))ee->next();
      len -= ee->size();
    } break;
    case ExtensionType::key_share: {
      exMap.insert({ex->extension_type, ex->extension_data()});
      auto ee = (Extension<KeyShareEntry> *)ex;
      ex = (decltype(ex))ee->next();
      len -= ee->size();
    } break;
    default:
      assert(false);
      break;
    }
  }

  return exMap;
}