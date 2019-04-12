#include <cstdlib>

#include "tls.h"
#include "transportimpl.h"
#include "packimpl.h"

MessagePack::MessagePack(size_t len) : _ring(len) {}

bool MessagePack::pack(BufT &buf, const char *p,
                 unsigned short len) {
   assert(buf.len == 0 && buf.base == nullptr);
  if (!p || !len) {
    assert(false);
    return false;
  }

  int head_len = sizeof(unsigned short);
  if (len + head_len > _ring.capacity()) {
    assert(false);
    return false;
  }

  buf = allocBuf(head_len + len);
  *(unsigned short *)buf.base = len;
  memcpy(buf.base + head_len, p, len);

  return true;
}

size_t MessagePack::feed(const char *data, size_t bytes) {
  return _ring.write(data, bytes);
}

bool MessagePack::unpack(BufT &buf) {
  assert(buf.len == 0 && buf.base == nullptr);

  int head_len = sizeof(uint32_t);
  uint32_t body_len = 0;
  if (!_ring.peek((char*)&body_len, sizeof(body_len))) {
    return false;
  }
  body_len = NTOHL(body_len);
  body_len &= 0x00FFFFFF;

  auto len = body_len + head_len;
  if (_ring.size() < len) {
    return false;
  }
  if (_ring.capacity() < len) {
    assert(false);
    return false;
  }

  buf = allocBuf(len);
  _ring.read((char*)buf.base, len);

  return true;
}

std::unique_ptr<PackInterface> MessagePack::clone() const  {
  return std::make_unique<MessagePack>(_ring.capacity());
}

// --

TSLPlaintextPack::TSLPlaintextPack(size_t len) : _ring(len) {}

bool TSLPlaintextPack::pack(BufT &buf, const char *p,
                 unsigned short len)  {
  assert(buf.len == 0 && buf.base == nullptr);
  if (!p || !len) {
    assert(false);
    return false;
  }

  int head_len = sizeof(unsigned short);
  if (len + head_len > _ring.capacity()) {
    assert(false);
    return false;
  }

  buf = allocBuf(head_len + len);
  *(unsigned short *)buf.base = len;
  memcpy(buf.base + head_len, p, len);

  return true;
}

size_t TSLPlaintextPack::feed(const char *data, size_t bytes)  {
  return _ring.write(data, bytes);
}

bool TSLPlaintextPack::unpack(BufT &buf)  {
  assert(buf.len == 0 && buf.base == nullptr);

  int head_len = sizeof(unsigned short);
  unsigned short body_len = 0;
  if (!_ring.peekAsUnsignedShort(&body_len)) {
    return false;
  }
  if (_ring.size() < (head_len + body_len)) {
    return false;
  }
  if (_ring.capacity() < body_len + head_len) {
    assert(false);
    return false;
  }

  _ring.advance(head_len);
  buf = allocBuf(body_len);
  _ring.read((char*)buf.base, body_len);

  return true;
}

std::unique_ptr<PackInterface> TSLPlaintextPack::clone() const  {
  return std::make_unique<TSLPlaintextPack>(_ring.capacity());
}
