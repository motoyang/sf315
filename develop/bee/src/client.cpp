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
#include "client.h"

// --

// --

Client::Client()
    : _rl(std::make_unique<RecordLayer>()),
      _channel(std::make_unique<Channel>()),
      _cryptocenter(std::make_unique<Cryptocenter>()) {
  _status.push(Client::Status::START);
}

void Client::start() {
  std::vector<uint8_t> buf(10024, 0);
  sayHello(hello(buf));
}

const Handshake *Client::hello(std::vector<uint8_t> &buf) const {
  std::unique_ptr<secure::RandomNumberGenerator> rng(
      new secure::AutoSeeded_RNG);

  auto hs_frame = (Handshake *)buf.data();
  hs_frame->msg_type(HandshakeType::client_hello);
  hs_frame->length(259);

  auto ch = (ClientHello *)hs_frame->message();
  ch->legacy_version = 0x0303;
  rng->randomize(ch->random, sizeof(ch->random));
  ch->legacy_session_id()->len(0);
  ch->cipher_suites()->len(10);
  auto cs = ch->cipher_suites()->data();
  cs[0] = HTONS(0x1301);
  cs[1] = HTONS(0x1302);
  cs[2] = HTONS(0x1303);
  cs[3] = HTONS(0x1304);
  cs[4] = HTONS(0x1305);
  ch->legacy_compression_methods()->len(1);
  ch->legacy_compression_methods()->data()[0] = 0;

  auto ex1 = (Extension<ClientSupportedVersions> *)ch->extensions()->data();
  ex1->extension_type = ExtensionType::supported_versions;
  auto csv = ex1->extension_data();
  csv->len(4);
  auto pv = csv->data();
  pv[0] = HTONS(0x0304);
  pv[1] = HTONS(0x0305);

  auto ex2 = (Extension<NamedGroupList> *)ex1->next();
  ex2->extension_type = ExtensionType::supported_groups;
  auto ngl = ex2->extension_data();
  ngl->len(6);
  ngl->data()[0] = NamedGroup::ffdhe2048;
  ngl->data()[1] = NamedGroup::ffdhe3072;
  ngl->data()[2] = NamedGroup::secp256r1;

  auto ex3 = (Extension<KeyShareClientHello> *)ex2->next();
  ex3->extension_type = ExtensionType::key_share;
  auto ksch = ex3->extension_data();
  // ksch->len(8);
  auto kse1 = ksch->data();
  kse1->group = NamedGroup::ffdhe2048;
  auto pk = _cryptocenter->privateKey(kse1->group)->public_value();
  kse1->key_exchange()->len(pk.size());
  auto key = kse1->key_exchange()->data();
  MemoryInterface::get()->copy(key, pk.data(), pk.size());

  auto kse2 = kse1->next();
  kse2->group = NamedGroup::ffdhe3072;
  kse2->key_exchange()->len(16);
  key = kse2->key_exchange()->data();
  int ii = 0;
  std::for_each(key, key + kse2->key_exchange()->len(),
                [&ii](auto &c) { c = ii++; });

  auto kse3 = kse2->next();
  kse3->group = NamedGroup::secp256r1;
  kse3->key_exchange()->len(3200);
  key = kse3->key_exchange()->data();
  for (int i = 0; i < kse3->key_exchange()->len(); ++i) {
    key[i] = i;
  }
  ksch->len(kse1->size() + kse2->size() + kse3->size());

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