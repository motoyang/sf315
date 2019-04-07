#include <iostream>

#include "transportimpl.h"
#include "packimpl.h"
#include "cryptography.h"
#include "recordlayer.h"

// --

RecordLayer::RecordLayer() : _rng(new secure::AutoSeeded_RNG),
  _packInterface(std::make_unique<MessagePack>(4096))
 {}

std::list<TLSPlaintext *>
RecordLayer::fragment(ContentType ct, const uint8_t *data, uint32_t len) const {
  std::list<TLSPlaintext *> l;
  while (len > 0) {
    auto buf_len = len > PlaintextMaxLength ? PlaintextMaxLength : len;
    len -= buf_len;
    auto p = TLSPlaintext::alloc(ct, PV, data, buf_len);
    l.push_back(p);
    data += buf_len;
  }
  return l;
}

std::list<std::vector<uint8_t>>
RecordLayer::fragmentWithPadding(ContentType ct, const uint8_t *data,
                                 uint32_t len) const {
  constexpr auto maxBufLen = PlaintextMaxLength + sizeof(ContentType);

  uint16_t len_of_padding = 0;
  std::list<std::vector<uint8_t>> l;
  while (len > 0) {
    auto buf_len = 0;
    if (len > maxBufLen) {
      buf_len = maxBufLen;
    } else {
      if (ct != ContentType::application_data || ApplicationDataPadding) {
        _rng->randomize((uint8_t *)&len_of_padding, sizeof(len_of_padding));
        len_of_padding = 1 + (len_of_padding % (maxBufLen - len));
      }
      buf_len = len;
    }
    len -= buf_len;
    auto v = TLSPlaintext::alloc(ct, PV, data, buf_len, len_of_padding);
    l.push_back(v);
  }
  return l;
}

std::list<std::vector<uint8_t>> RecordLayer::feed(const uint8_t *data, size_t len) const {
  std::list<std::vector<uint8_t>> list;
  auto p = (const char*)data;
  auto remain = len;
  do {
    int writed = _packInterface->feed(p, remain);
    remain -= writed;
    p += writed;

    while (true) {
      // 解析出每个包
      BufT b = {0};
      if (!_packInterface->unpack(b)) {
        break;
      }
      list.emplace_back(b.base, b.base + b.len);
    }
  } while (remain > 0);
  return list;
}
