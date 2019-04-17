#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>

#include "nanolog.hpp"
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
        _cryptocenter(std::make_unique<Cryptocenter>(true)) {}

  std::vector<uint8_t> _clientHello;
  std::stack<Status> _status;
  std::unique_ptr<VersionSupported> _vs;
  std::unique_ptr<RecordLayer> _rl;
  std::unique_ptr<Cryptocenter> _cryptocenter;
  std::unique_ptr<Channel> _channel;
};

// --

// 根据收到的帧，做顶层的分发
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
          // 首先要确定加密算法
          _impl->_cryptocenter->cipherSuitInit(*sh->cipher_suite());

          // 更新hash
          _impl->_cryptocenter->hashFun()->update(_impl->_clientHello.data(),
                                                  _impl->_clientHello.size());
          _impl->_cryptocenter->hashFun()->update((const uint8_t *)hs,
                                                  hs->size());

          received(sh);
        }
      } break;
      case HandshakeType::encrypted_extensions: {
        auto ee = (EncryptedExtensions *)hs->message();
        received(ee);
      } break;

      case HandshakeType::certificate_request: {

      } break;

      case HandshakeType::certificate: {

      } break;

      case HandshakeType::certificate_verify: {

      } break;

      case HandshakeType::finished: {

      } break;

      case HandshakeType::key_update: {

      } break;

      case HandshakeType::new_session_ticket: {

      } break;

      default:
        assert(false);
        break;
      }
    }
  } break;

  case ContentType::application_data: {

  } break;

  case ContentType::alert: {
    assert(sizeof(Alert) == len);
    auto alert = (Alert *)p;
    NLOG_CRIT << "Client received alert: " << alert->description;

    // TBD!!! notify to application level

  } break;

  case ContentType::change_cipher_spec:
  case ContentType::invalid:
  default:
    assert(false);
    break;
  }
}

void Client::received(const ServerHello *sh) {
  auto exMap = extensionsCheck(sh->extensions());

  // 根据Server返回的public key，产生ecdhc key。这就是密钥交换的结果。
  auto kse = (KeyShareEntry *)exMap.at(ExtensionType::key_share);
  auto len = kse->key_exchange()->len();
  auto data = kse->key_exchange()->data();
  std::vector<uint8_t> publicKey(data, data + len);
  auto b = _impl->_cryptocenter->driveKey(kse->group, publicKey);

  _impl->_cryptocenter->driveHkdf();

  _impl->_status.pop();
  _impl->_status.push(Client::Status::WAIT_EE);
}

void Client::received(const EncryptedExtensions *ee) {
  std::cout << hex2section(secure::hex_encode((const uint8_t *)ee, ee->size()))
            << std::endl;

  _impl->_status.pop();
  _impl->_status.push(Client::Status::WAIT_EE);
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
  sendFragment(ContentType::handshake, (const uint8_t *)hs, hs->size());

  _impl->_status.pop();
  _impl->_status.push(Client::Status::WAIT_SH);
}

void Client::sayAlert(AlertDescription desc, AlertLevel level) {
  Alert alert{level, desc};
  sendFragment(ContentType::alert, (const uint8_t *)&alert, sizeof(alert));

  NLOG_CRIT << "client send alter: " << desc;
}

void Client::sayAppdata(const uint8_t *p, size_t len) {
  sendFragment(ContentType::application_data, p, len);
}

void Client::sendFragment(ContentType ct, const uint8_t *p, size_t len) const {
  Client::Status status = _impl->_status.top();

  switch (status) {
  case Client::Status::START:
  case Client::Status::WAIT_SH: {
    auto lv = _impl->_rl->fragment(ct, p, len);
    for (const auto &v : lv) {
      _impl->_channel->send((const uint8_t *)v.data(), v.size());
    }
  } break;
  case Client::Status::WAIT_EE:
  case Client::Status::WAIT_CERT_CR:
  case Client::Status::WAIT_CERT:
  case Client::Status::WAIT_CV:
  case Client::Status::WAIT_FINISHED:
  case Client::Status::CONNECTED: {
    auto lv = _impl->_rl->fragmentWithPadding(ct, p, len);
    for (auto &v : lv) {
      _impl->_cryptocenter->clientCrypto(v);
      _impl->_channel->send(v.data(), v.size());
    }
  } break;
  default:
    assert(false);
    break;
  }
}

bool Client::recvFragment(ContentType &ct,
                          secure::secure_vector<uint8_t> &buf) const {
  // v可能是TSLPlaintext或者TSLCiphertext，头都是相同的。
  auto plaintext = (TLSPlaintext *)buf.data();
  ct = plaintext->type;

  auto status = _impl->_status.top();
  if ((status == Client::Status::WAIT_SH) ||
      (status == Client::Status::START)) {
    // 不需要解密
  } else {
    // v是TSLCiphertext，要解密后处理
    _impl->_cryptocenter->serverDecrypto(buf);

    // 剔除padding的zero
    auto ct_addr = std::find_if(buf.crbegin(), buf.crend(),
                                [](uint8_t c) { return c > 0; });
    if (ct_addr == buf.crend()) {
      assert(false);
      return false;
    }
    ct = ContentType(*ct_addr);
    auto distance = std::distance(ct_addr, buf.crend());
    buf.resize(distance - 1); // -1是为了排除TLSInnerPlaintext.type

    // 根据解密后的buf.size()，更新TLSPlaintext的长度
    plaintext = (TLSPlaintext *)buf.data();
    plaintext->length((buf.size() - sizeof(*plaintext)));
  }

  return true;
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

    // 首先对接收到的fragment进行解密（也可能不需要解密），删除padding，
    // 取出真正的ContentType。
    ContentType ct = ContentType::invalid;
    if (!recvFragment(ct, v)) {
      continue;
    }

    // 从fragment拼接为完整的帧
    auto plaintext = (const TLSPlaintext *)v.data();
    auto fragment = plaintext->fragment();
    auto len = plaintext->length();
    auto l = _impl->_rl->feed(fragment, len);
    assert(l.size() <= 1);

    // 处理每个帧
    for (const auto &m : l) {
      received(ct, m.data(), m.size());
    }
  }
}
