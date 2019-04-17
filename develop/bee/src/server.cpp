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

struct Server::Impl {
  Impl()
      : _vs(std::make_unique<VersionSupported>()),
        _rl(std::make_unique<RecordLayer>()),
        _channel(std::make_unique<Channel>()),
        _cryptocenter(std::make_unique<Cryptocenter>(false)) {}

  std::stack<Status> _status;
  std::unique_ptr<VersionSupported> _vs;
  std::unique_ptr<RecordLayer> _rl;
  std::unique_ptr<Cryptocenter> _cryptocenter;
  std::unique_ptr<Channel> _channel;
};

// --

Handshake *Server::hello(std::vector<uint8_t> &buf, const ClientHello *ch) {
  // 解析Extensions到map，以便后续使用
  auto exMap = extensionsCheck(ch->extensions());

  // 没有supported_versions扩展，就认为不是TLS1.3版本，不支持并发送alert。
  // from rfc8446: TLS 1.3 ClientHello messages always
  // contain extensions (minimally "supported_versions", otherwise, they
  // will be interpreted as TLS 1.2 ClientHello messages).
  if (ch->legacy_version != PV_TLS_1_2 ||
      exMap.find(ExtensionType::supported_versions) == exMap.end()) {
    // 中断握手过程。TLS1.3规定，legacy_version必须是PV_TLS_1_2(0x0303)，版本协商通过
    // SupportedVersions扩张完成。
    // from rfc8446: Servers MAY abort the handshake upon receiving a
    // ClientHello with legacy_version 0x0304 or later.

    // 发送protocol_version Alert
    sayAlert(AlertDescription::protocol_version);
    return nullptr;
  }

  // from rfc8446: For every TLS 1.3 ClientHello, this vector
  // MUST contain exactly one byte, set to zero, which corresponds to
  // the "null" compression method in prior versions of TLS.
  if (ch->legacy_compression_methods()->len() > 1) {
    // If a TLS 1.3 ClientHello is received with any other value in this
    // field, the server MUST abort the handshake with an
    // "illegal_parameter" alert.
    sayAlert(AlertDescription::illegal_parameter);
    return nullptr;
  }

  // 开始Handshake
  auto hs = (Handshake *)buf.data();
  hs->msg_type(HandshakeType::server_hello);

  // 开始ServerHello
  auto sh = (ServerHello *)hs->message();
  sh->legacy_version = PV_TLS_1_2;
  _impl->_cryptocenter->rng()->randomize(sh->random, sizeof(sh->random));

  // 原样返回ClientHello.legacy_session
  // from RFC8446: The contents of the client’s legacy_session_id field.
  auto lsie = sh->legacy_session_id_echo();
  lsie->len(ch->legacy_session_id()->len());
  for (auto i = 0; i < lsie->len(); ++i) {
    lsie->data()[i] = ch->legacy_session_id()->data()[i];
  }

  if (!_impl->_cryptocenter->select(sh->cipher_suite(), ch->cipher_suites())) {
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

  if (!_impl->_vs->selected(ex1->extension_data(),
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
  const KeyShareClientHello *ksch = nullptr;
  if (exMap.find(ExtensionType::key_share) != exMap.end()) {
    ksch = (KeyShareClientHello *)exMap.at(ExtensionType::key_share);
  }
  if (!_impl->_cryptocenter->select(kse, ksch)) {
    NLOG_WARN << "Server can't found matched KeyShareEntry in ClientHello, so "
                 "HelloRetryRequest will been sent.";
    sh->helloRetryRequest();
  }
  /*
    if (sh->isHelloRetryRequest()) {
      auto ex3 = (Extension<Cookie> *)ex2->next();
      ex3->extension_type = ExtensionType::cookie;
      auto cookie = ex3->extension_data();
      auto hashValue =
          _impl->_cryptocenter->hashFun()->final((const uint8_t *)ch,
    ch->size()); cookie->len(hashValue.size());
      MemoryInterface::get()->copy(cookie->data(), hashValue.data(),
                                   hashValue.size());

      sh->extensions()->len(ex1->size() + ex2->size() + ex3->size());
    } else {
      sh->extensions()->len(ex1->size() + ex2->size());
    }
  */
  sh->extensions()->len(ex1->size() + ex2->size());
  hs->length(sh->size());

  assert(buf.size() >= hs->size());
  buf.resize(hs->size());
  return (Handshake *)buf.data();
}

void Server::sayHello(const Handshake *hs) {
  sendFragment(ContentType::handshake, (const uint8_t *)hs, hs->size());
  auto sh = (ServerHello *)hs->message();
  if (sh->isHelloRetryRequest()) {
    auto hashFun = _impl->_cryptocenter->hashFun();
    ClientHello1Hash ch1Hash{
        HandshakeType::message_hash, {0, 0}, (uint8_t)hashFun->output_length()};
    auto hashValue = hashFun->final();

    hashFun->update((const uint8_t *)&ch1Hash, sizeof(ch1Hash));
    hashFun->update(hashValue);
  } else {
    _impl->_cryptocenter->hashFun()->update((const uint8_t *)hs, hs->size());
    _impl->_cryptocenter->driveHkdf();

    _impl->_status.pop();
    _impl->_status.push(Server::Status::NEGOTIATED);
  }
}

void Server::sayEncryptedExtensions() {
  std::vector<uint8_t> buf(64, 0);
  auto hs = (Handshake *)buf.data();
  hs->msg_type(HandshakeType::encrypted_extensions);

  auto ee = (EncryptedExtensions *)hs->message();
  ee->len(0);

  hs->length(ee->size());
  assert(buf.size() >= hs->size());
  buf.resize(hs->size());
  sendFragment(ContentType::handshake, buf.data(), buf.size());

  // TBF!!!
  _impl->_status.pop();
//  _impl->_status.push(Server::Status::WAIT_EOED);
}

void Server::sayAlert(AlertDescription desc, AlertLevel level) {
  Alert alert{level, desc};
  sendFragment(ContentType::alert, (const uint8_t *)&alert, sizeof(alert));
  NLOG_CRIT << "server send alter: " << desc;
}

void Server::sendFragment(ContentType ct, const uint8_t *p, size_t len) const {
  Server::Status status = _impl->_status.top();

  switch (status) {
  case Server::Status::START:
  case Server::Status::RECVD_CH: {
    auto lv = _impl->_rl->fragment(ct, p, len);
    for (const auto &v : lv) {
      _impl->_channel->send((const uint8_t *)v.data(), v.size());
    }
  } break;

  case Server::Status::NEGOTIATED:
  case Server::Status::WAIT_EOED:
  case Server::Status::WAIT_FLIGHT2:
  case Server::Status::WAIT_CERT:
  case Server::Status::WAIT_CV:
  case Server::Status::WAIT_FINISHED:
  case Server::Status::CONNECTED: {
    auto lv = _impl->_rl->fragmentWithPadding(ct, p, len);
    for (auto &v : lv) {
      _impl->_cryptocenter->serverCrypto(v);
      _impl->_channel->send(v.data(), v.size());
    }
  } break;

  default:
    assert(false);
    break;
  }
}

bool Server::recvFragment(ContentType &ct,
                          secure::secure_vector<uint8_t> &buf) const {
  // v可能是TSLPlaintext或者TSLCiphertext，头都是相同的。
  auto plaintext = (TLSPlaintext *)buf.data();
  ct = plaintext->type;

  auto status = _impl->_status.top();
  if ((status == Server::Status::START) ||
      (status == Server::Status::RECVD_CH)) {
    // 不需要解密
  } else {
    // v是TSLCiphertext，要解密后处理
    _impl->_cryptocenter->clientDecrypto(buf);

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

// 根据收到的包，做顶层的分发
void Server::received(ContentType ct, const uint8_t *p, size_t len) {
  switch (ct) {
  case ContentType::handshake: {
    while (len > 0) {
      auto hsClientHello = (Handshake *)p;
      len -= hsClientHello->size();
      p += len;

      switch (hsClientHello->msg_type()) {
      case HandshakeType::client_hello: {
        auto ch = (ClientHello *)hsClientHello->message();

        std::vector<uint8_t> buf(4096);
        if (auto hsServerHello = hello(buf, ch); hsServerHello) {
          _impl->_cryptocenter->hashFun()->update((uint8_t *)hsClientHello,
                                                  hsClientHello->size());
          sayHello(hsServerHello);
          sayEncryptedExtensions();
        }
      } break;

      default:
        break;
      }
    }
  } break;
  case ContentType::application_data:
    break;
  case ContentType::alert: {
    assert(sizeof(Alert) == len);
    auto alert = (Alert *)p;
    NLOG_CRIT << "Server received alert: " << alert->description;

    // TBD!!! notify to application level

  } break;
  case ContentType::change_cipher_spec:
  case ContentType::invalid:
  default:
    assert(false);
    break;
  }
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

// --

Server::Server() : _impl(std::make_unique<Server::Impl>()) {
  _impl->_status.push(Server::Status::START);
}

Server::~Server() {}

Channel *Server::channel() const { return _impl->_channel.get(); }

void Server::run() {
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
