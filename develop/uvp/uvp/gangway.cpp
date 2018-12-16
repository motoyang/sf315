#include <string>
#include <iostream>
#include <sstream>
#include <atomic>

#include <uv.h>
#include <types.hpp>
#include <ringbuffer.hpp>
#include <utilites.hpp>
#include <misc.hpp>

#include <gangway.hpp>

// --

Packet::Packet() {
  _buf.base = nullptr;
  _buf.len = 0;
}

Packet::Packet(const std::string &p, BufT buf) : _peer(p), _buf(buf) {}

Packet::~Packet() { freeBuf(_buf); }

Packet::Packet(Packet &&p) {
  _peer = std::move(p._peer);
  _buf = p._buf;
  p._buf.base = nullptr;
  p._buf.len = 0;
}

Packet &Packet::operator=(Packet &&p) {
  _peer = std::move(p._peer);
  _buf = p._buf;
  p._buf.base = nullptr;
  p._buf.len = 0;
  return *this;
}

BufT Packet::releaseBuf() {
  BufT b = _buf;
  _buf.base = nullptr;
  _buf.len = 0;
  return b;
}

// --

Codec::Codec(char mark) : _mark(mark), _ring(45) {}

// unsigned short Codec::size() const {
//   return _ring.capacity();
// }

RingBuffer& Codec::ringBuffer() const {
  return _ring;
}

bool Codec::encode(BufT& buf, const char *p, unsigned short len) {
  UVP_ASSERT(buf.len == 0 && buf.base == nullptr);
  if (!p || !len) {
    UVP_ASSERT(false);
    return false;
  }

  char head[16] = {0};
  std::snprintf(head, sizeof(head), "%d%c", (int)len, _mark);
  int head_len = std::strlen(head);
  if (len + head_len > _ring.capacity()) {
    UVP_ASSERT(false);
    return false;
  }

  buf = allocBuf(head_len + len);
  memcpy(buf.base, head, head_len);
  memcpy(buf.base + head_len, p, len);

  return true; 
}

bool Codec::decode(BufT& buf) {
  UVP_ASSERT(buf.len == 0 && buf.base == nullptr);

  char *body = _ring.search(_mark);
  if (!body) {
    return false;
  }

  int head_len = _ring.offset(body);
  if (head_len < 1) {
    UVP_ASSERT(false);
    return false;
  }

  char header[16];
  _ring.peek(header, head_len);
  header[head_len] = '\0';
  int body_len = atoi(header);
  if (body_len < 1) {
    UVP_ASSERT(false);
    return false;
  }

  if (_ring.size() < (head_len + body_len + 1)) {
    return false;
  }

  _ring.advance(head_len + 1);
  buf = allocBuf(body_len);
  _ring.read(buf.base, body_len);

  return true;
}

// --

Codec2::Codec2(): _ring(48) {}

// unsigned short Codec2::size() const {
//   return _ring.capacity();
// }

RingBuffer& Codec2::ringBuffer() const {
  return _ring;
}

bool Codec2::encode(BufT& buf, const char *p, unsigned short len) {
  UVP_ASSERT(buf.len == 0 && buf.base == nullptr);
  if (!p || !len) {
    UVP_ASSERT(false);
    return false;
  }

  int head_len = sizeof(unsigned short);
  if (len + head_len > _ring.capacity()) {
    UVP_ASSERT(false);
    return false;
  }

  buf = allocBuf(head_len + len);
  *(unsigned short*)buf.base = len;
  memcpy(buf.base + head_len, p, len);

  return true; 
}

bool Codec2::decode(BufT& buf) {
  UVP_ASSERT(buf.len == 0 && buf.base == nullptr);

  int head_len = sizeof(unsigned short);
  unsigned short body_len = 0;
  if (!_ring.peekAsUnsignedShort(&body_len)) {
    return false;
  }
  if (_ring.size() < (head_len + body_len)) {
    return false;
  }
  if (_ring.capacity() < body_len + head_len) {
    UVP_ASSERT(false);
    return false;
  }

  _ring.advance(head_len);
  buf = allocBuf(body_len);
  _ring.read(buf.base, body_len);

  return true;
}
