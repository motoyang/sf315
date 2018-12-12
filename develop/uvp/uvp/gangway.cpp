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

Codec::Codec(char mark) : _mark(mark) {}

unsigned short Codec::size() const {
  return 45;
}

BufT Codec::encode(const char *p, unsigned short len) {
  BufT b;
  b.len = 0;
  b.base = 0;
  if (!p) {
    assert(false);
    return b;
  }

  char buf[10] = {0};
  std::snprintf(buf, sizeof(buf), "%d%c", (int)len, _mark);
  int head = std::strlen(buf);
  if (len + head > size()) {
    assert(false);
    return b;
  }


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

// --

Codec2::Codec2(char mark): _mark(mark) {}

unsigned short Codec2::size() const {
  return 488;
}

BufT Codec2::encode(const char *p, unsigned short len) {
  BufT b;
  b.len = 0;
  b.base = 0;
  if (!p || !len) {
    assert(false);
    return b;
  }

  char head[16] = {0};
  std::snprintf(head, sizeof(head), "%d%c", (int)len, _mark);
  int head_len = std::strlen(head);
  if (len + head_len > size()) {
    assert(false);
    return b;
  }

  b = allocBuf(head_len + len);
  memcpy(b.base, head, head_len);
  memcpy(b.base + head_len, p, len);

  return b; 
}

BufT Codec2::decode(RingBuffer *ringbuffer) {
  BufT r;
  r.len = 0;
  r.base = nullptr;

  char *body = ringbuffer->search(_mark);
  if (!body) {
    return r;
  }

  int head_len = ringbuffer->offset(body);
  if (head_len < 1) {
    assert(false);
    return r;
  }

  char header[16];
  ringbuffer->peek(header, head_len);
  header[head_len] = '\0';
  int body_len = atoi(header);
  if (body_len < 1) {
    assert(false);
    return r;
  }

  if (ringbuffer->size() < (head_len + body_len + 1)) {
    return r;
  }

  r = allocBuf(body_len);
  ringbuffer->advance(head_len + 1);
  ringbuffer->read(r.base, body_len);

  return r;
}

/*
BufT Codec2::encode(const char *p, unsigned short len) {
  int head_len = sizeof(len);
  int mark_len = sizeof(_mark);

  BufT b;
  b.len = 0;
  b.base = 0;
  if ((head_len + mark_len + len > size()) || !p) {
    assert(false);
    return b;
  }

  b = allocBuf(head_len + mark_len + len);
  *(unsigned short*)b.base = len;
  *(char*)(b.base+head_len) = _mark;
  memcpy(b.base + head_len + mark_len, p, len);

  return b;
}

BufT Codec2::decode(RingBuffer *ringbuffer) {
  BufT r;
  r.len = 0;
  r.base = nullptr;

  char *body = ringbuffer->search(_mark);
  if (!body) {
    return r;
  }
  int head_len = ringbuffer->offset(body);
  if (head_len != sizeof(short)) {
    // ringbuffer->advance(head_len + 1);
    assert(false);
    return r;
  }

  unsigned short body_len = 0;
  ringbuffer->peek((char*)&body_len, head_len);
  if (ringbuffer->size() < (head_len + body_len + 1)) {
    return r;
  }

  r = allocBuf(body_len);
  ringbuffer->advance(head_len + 1);
  ringbuffer->read(r.base, body_len);

  return r;
}
*/