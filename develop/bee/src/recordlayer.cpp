#include <iostream>

#include "memoryimpl.h"
#include "transportimpl.h"
#include "packimpl.h"
#include "cryptography.h"
#include "recordlayer.h"

// --

static secure::secure_vector<uint8_t> allocPlaintext(ContentType ct, ProtocolVersion pv,
                                  const uint8_t *data, uint16_t len) {
  auto mi = MemoryInterface::get();
  secure::secure_vector<uint8_t> buf(sizeof(TLSPlaintext) + len, 0);
  auto r = (TLSPlaintext *)buf.data();
  r->type = ct;
  r->legacy_record_version = pv;
  r->length(len);
  mi->copy(r->fragment(), data, len);

  return buf;
}

static secure::secure_vector<uint8_t> allocPlaintext(ContentType ct, ProtocolVersion pv,
                                         const uint8_t *data, uint16_t len,
                                         uint16_t length_of_padding) {
  auto mi = MemoryInterface::get();
  auto buf_len = sizeof(TLSPlaintext) + len + sizeof(ct) + length_of_padding;
  secure::secure_vector<uint8_t> buf(buf_len);
  auto r = (TLSPlaintext *)buf.data();
  r->type = ct;
  r->legacy_record_version = pv;
  r->length(len);
  mi->copy(r->fragment(), data, len);
  r->innerType(ct);
  mi->set(r->innerZeros(), 0, length_of_padding);

  return buf;
}

// --

constexpr static uint16_t PlaintextMaxLength = 0xFF; // 0x3FFF;
constexpr static ProtocolVersion PV = 0x0303;
constexpr static bool ApplicationDataPadding = false;

struct RecordLayer::Impl {
  Impl()
      : _rng(new secure::AutoSeeded_RNG),
        _packInterface(std::make_unique<MessagePack>(4096)) {}

  std::unique_ptr<secure::RandomNumberGenerator> _rng;
  std::unique_ptr<PackInterface> _packInterface;
};

// --

RecordLayer::RecordLayer() : _impl(std::make_unique<RecordLayer::Impl>()) {}

RecordLayer::~RecordLayer() {}

std::list<secure::secure_vector<uint8_t>>
RecordLayer::fragment(ContentType ct, const uint8_t *data, uint32_t len) const {
  std::list<secure::secure_vector<uint8_t>> l;
  while (len > 0) {
    auto buf_len = len > PlaintextMaxLength ? PlaintextMaxLength : len;
    len -= buf_len;
    auto v = allocPlaintext(ct, PV, data, buf_len);
    l.push_back(v);
    data += buf_len;
  }
  return l;
}

std::list<secure::secure_vector<uint8_t>>
RecordLayer::fragmentWithPadding(ContentType ct, const uint8_t *data,
                                 uint32_t len) const {
  constexpr auto maxBufLen = PlaintextMaxLength + sizeof(ContentType);

  uint16_t len_of_padding = 0;
  std::list<secure::secure_vector<uint8_t>> l;
  while (len > 0) {
    auto buf_len = 0;
    if (len > maxBufLen) {
      buf_len = maxBufLen;
    } else {
      if (ct != ContentType::application_data || ApplicationDataPadding) {
        _impl->_rng->randomize((uint8_t *)&len_of_padding, sizeof(len_of_padding));
        len_of_padding = 1 + (len_of_padding % (maxBufLen - len));
      }
      buf_len = len;
    }
    len -= buf_len;
    auto v = allocPlaintext(ct, PV, data, buf_len, len_of_padding);
    l.push_back(v);
  }
  return l;
}

std::list<std::vector<uint8_t>> RecordLayer::feed(const uint8_t *data,
                                                  size_t len) const {
  std::list<std::vector<uint8_t>> list;
  auto p = (const char *)data;
  auto remain = len;
  do {
    int writed = _impl->_packInterface->feed(p, remain);
    remain -= writed;
    p += writed;

    while (true) {
      // 解析出每个包
      BufT b = {0};
      if (!_impl->_packInterface->unpack(b)) {
        break;
      }
      list.emplace_back(b.base, b.base + b.len);
    }
  } while (remain > 0);
  return list;
}
