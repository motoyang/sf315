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
#include "client.h"

// --

struct Client::Impl {
  Impl()
      : _vs(std::make_unique<VersionSupported>()),
        _rl(std::make_unique<RecordLayer>()),
        _channel(std::make_unique<Channel>()),
        _cryptocenter(std::make_unique<Cryptocenter>()) {}

  std::vector<uint8_t> _clientHello;
  std::stack<Status> _status;
  std::unique_ptr<VersionSupported> _vs;
  std::unique_ptr<RecordLayer> _rl;
  std::unique_ptr<Cryptocenter> _cryptocenter;
  std::unique_ptr<Channel> _channel;

  std::vector<uint8_t> _key;
};

// --

// 根据收到的包，做顶层的分发
void Client::received(ContentType ct, const uint8_t *p, size_t len) {
  switch (ct) {
  case ContentType::handshake: {
    while (len > 0) {
      auto hs = (Handshake *)p;
      len -= hs->size();
      p += len;

      switch (hs->msg_type()) {
      case HandshakeType::server_hello: {
        auto sh = (ServerHello *)hs->message();
        if (sh->isHelloRetryRequest()) {
          auto ch = hello(_impl->_clientHello, sh);
          sayHello(ch);
        } else {
          received(sh);
        }
      } break;

      default:
        break;
      }
    }
  } break;
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

void Client::received(const ServerHello *sh) {
  _impl->_cryptocenter->cipherSuitInit(*sh->cipher_suite());
  _impl->_cryptocenter->hashUpdate(_impl->_clientHello.data(),
                                   _impl->_clientHello.size());
  _impl->_cryptocenter->hashUpdate((uint8_t *)sh, sh->size());

  auto exMap = extensionsCheck(sh->extensions());

  // 根据Server返回的public key，产生ecdhc key。这就是密钥交换的结果。
  auto kse = (KeyShareEntry *)exMap.at(ExtensionType::key_share);
  auto len = kse->key_exchange()->len();
  auto data = kse->key_exchange()->data();
  std::vector<uint8_t> publicKey(data, data + len);
  auto b = _impl->_cryptocenter->driveKey(kse->group, publicKey);

  _impl->_cryptocenter->driveHkdf();
}

const Handshake *Client::hello(std::vector<uint8_t> &buf) const {
  auto hs = (Handshake *)buf.data();
  hs->msg_type(HandshakeType::client_hello);

  auto ch = (ClientHello *)hs->message();
  // In TLS 1.3, the client indicates its version preferences in the
  // "supported_versions" extension (Section 4.2.1) and the
  // legacy_version field MUST be set to 0x0303, which is the version
  // number for TLS 1.2.
  ch->legacy_version = PV_TLS_1_2;
  _impl->_cryptocenter->rng()->randomize(ch->random, sizeof(ch->random));
  ch->legacy_session_id()->len(0);
  _impl->_cryptocenter->support(ch->cipher_suites());
  ch->legacy_compression_methods()->len(1);
  ch->legacy_compression_methods()->data()[0] = 0;

  auto ex1 = (Extension<ClientSupportedVersions> *)ch->extensions()->data();
  ex1->extension_type = ExtensionType::supported_versions;
  _impl->_vs->supported(ex1->extension_data());

  auto ex2 = (Extension<NamedGroupList> *)ex1->next();
  ex2->extension_type = ExtensionType::supported_groups;
  _impl->_cryptocenter->support(ex2->extension_data());

  auto ex3 = (Extension<KeyShareClientHello> *)ex2->next();
  ex3->extension_type = ExtensionType::key_share;
  auto ksch = ex3->extension_data();
  _impl->_cryptocenter->support(ksch);

  ch->extensions()->len(ex1->size() + ex2->size() + ex3->size());
  hs->length(ch->size());

  assert(buf.size() >= hs->size());
  buf.resize(hs->size());
  return (const Handshake *)buf.data();
}

// 根据Server发来的HelloRetryRequest，重新发送ClientHello
const Handshake *Client::hello(std::vector<uint8_t> &buf,
                               const ServerHello *reques) const {
  // TBD!!!
}

void Client::sayHello(const Handshake *hs) {
  auto pl =
      _impl->_rl->fragment(ContentType::handshake, (uint8_t *)hs, hs->size());
  for (const auto &p : pl) {
    _impl->_channel->send((uint8_t *)p, p->size());
  }

  _impl->_status.pop();
  _impl->_status.push(Client::Status::WAIT_SH);
}

void Client::sayAppdata(const uint8_t *p, size_t len) {
  // TBF!!!
  auto pv =
      _impl->_rl->fragmentWithPadding(ContentType::application_data, p, len);
  for (const auto &v : pv) {
    auto cv = _impl->_cryptocenter->crypto(v, _impl->_key);
    _impl->_channel->send(cv.data(), cv.size());
  }
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

// --

Client::Client() : _impl(std::make_unique<Client::Impl>()) {
  _impl->_status.push(Client::Status::START);
}

Client::~Client() {}

Channel *Client::channel() const { return _impl->_channel.get(); }

void Client::start() {
  _impl->_clientHello.assign(1024 * 20, 0);
  sayHello(hello(_impl->_clientHello));
}

void Client::run() {
  for (auto v = _impl->_channel->recv(); v.size() > 0;
       v = _impl->_channel->recv()) {
    // v可能是TSLPlaintext或者TSLCiphertext，头都是相同的。
    auto plaintext = (const TLSPlaintext *)v.data();
    auto fragment = plaintext->fragment();
    auto len = plaintext->length();
    auto ct = plaintext->type;
    if (plaintext->cryptoFlag()) {
      // v是TSLCiphertext，要解密后处理
      v = _impl->_cryptocenter->decrypto(fragment, len, _impl->_key);

      // 剔除padding的zero
      auto ct_addr =
          std::find_if(v.crbegin(), v.crend(), [](uint8_t c) { return c > 0; });
      if (ct_addr == v.crbegin()) {
        assert(false);
        continue;
      }
      ct = ContentType(*ct_addr);
      auto distance = std::distance(ct_addr, v.crbegin());
      v.resize(distance - 1); // -1是为了排除TLSInnerPlaintext.type
      fragment = v.data();
      len = v.size();
    }
    auto l = _impl->_rl->feed(fragment, len);
    assert(l.size() <= 1);
    for (const auto &m : l) {
      received(ct, m.data(), m.size());
    }
  }
}
