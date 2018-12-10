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

// --

Codec::Codec(char mark) : _mark(mark) {}

BufT Codec::encode(const char *p, size_t len) {
  BufT b;
  b.len = 0;
  b.base = 0;
  if ((len > std::numeric_limits<short>::max()) || !p) {
    return b;
  }

  char buf[10] = {0};
  std::snprintf(buf, sizeof(buf), "%d%c", (int)len, _mark);
  int head = std::strlen(buf);

  b = allocBuf(head + len);
  memcpy(b.base, buf, len);
  memcpy(b.base + head, p, len);

  return b;
}

BufT Codec::decode(RingBuffer *ringbuffer) {
  BufT r;
  r.len = 0;
  r.base = nullptr;

  int head_len = 0;
  int body_len = 0;

  {
    char *body = ringbuffer->search(_mark);
    if (!body) {
      return r;
    }
    head_len = ringbuffer->offset(body);
  }

  {
    char header[20];
    ringbuffer->peek(header, head_len);
    header[head_len] = '\0';
    body_len = atoi(header);
    if (body_len <= 0) {
      return r;
    }
  }

  if (ringbuffer->size() < (head_len + body_len + 1)) {
    return r;
  }

  r = allocBuf(body_len);
  ringbuffer->advance(head_len + 1);
  ringbuffer->read(r.base, body_len);

  return r;
}
