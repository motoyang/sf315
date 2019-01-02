#pragma once

#include "concurrentqueue.h"
#include "blockingconcurrentqueue.h"
#include "ringbuffer.hpp"

#include <uvp.hpp>

// --

namespace uvplus {

struct Packet {
  std::string _peer;
  uvp::uv::BufT _buf;

  Packet() : _buf{0} {}
  Packet(const std::string &p, uvp::uv::BufT buf) : _peer(p), _buf(buf) {}
  ~Packet() {
    if (_buf.len) {
      uvp::freeBuf(_buf);
    }
  }

  Packet(const Packet &) = delete;
  Packet operator=(const Packet &) = delete;

  Packet(Packet &&p) {
    _peer = std::move(p._peer);
    _buf = p._buf;
    p._buf.base = nullptr;
    p._buf.len = 0;
  }

  Packet &operator=(Packet &&p) {
    _peer = std::move(p._peer);
    _buf = p._buf;
    p._buf.base = nullptr;
    p._buf.len = 0;
    return *this;
  }

  uvp::uv::BufT releaseBuf() {
    uvp::uv::BufT b = _buf;
    _buf.base = nullptr;
    _buf.len = 0;
    return b;
  }
};

// --

struct Gangway {
  moodycamel::BlockingConcurrentQueue<Packet> _upward;
  moodycamel::ConcurrentQueue<Packet> _downward;
};

struct GangwayInConnector {
  moodycamel::BlockingConcurrentQueue<uvp::uv::BufT> _upward;
  moodycamel::ConcurrentQueue<uvp::uv::BufT> _downward;
};

// --

class Codec {
  char _mark;
  mutable RingBuffer _ring;

public:
  Codec(char mark) : _mark(mark), _ring(45) {}

  RingBuffer &ringBuffer() const { return _ring; }

  bool encode(uvp::uv::BufT &buf, const char *p, unsigned short len) {
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

    buf = uvp::allocBuf(head_len + len);
    memcpy(buf.base, head, head_len);
    memcpy(buf.base + head_len, p, len);

    return true;
  }

  virtual bool decode(uvp::uv::BufT &buf) {
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
    buf = uvp::allocBuf(body_len);
    _ring.read(buf.base, body_len);

    return true;
  }
};

class Codec2 {
  mutable RingBuffer _ring;

public:
  Codec2() : _ring(48) {}

  RingBuffer &ringBuffer() const { return _ring; }

  bool encode(uvp::uv::BufT &buf, const char *p, unsigned short len) {
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

    buf = uvp::allocBuf(head_len + len);
    *(unsigned short *)buf.base = len;
    memcpy(buf.base + head_len, p, len);

    return true;
  }

  bool decode(uvp::uv::BufT &buf) {
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
    buf = uvp::allocBuf(body_len);
    _ring.read(buf.base, body_len);

    return true;
  }
};

} // namespace uvplus
