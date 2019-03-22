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

struct NoCopyable {
  NoCopyable() = default;
  NoCopyable(const NoCopyable&) = delete;
  NoCopyable& operator=(const NoCopyable&) = delete;
};

struct PackInterface: NoCopyable {
  virtual ~PackInterface() = default;
  virtual bool pack(uvp::uv::BufT &buf, const char *p, unsigned short len) = 0;
  virtual size_t feed(const char *data, size_t bytes) = 0;
  virtual bool unpack(uvp::uv::BufT &buf) = 0;
  virtual std::unique_ptr<PackInterface> clone() const = 0;
};

// --

class Pack1: public PackInterface {
  char _mark;
  RingBuffer _ring;

public:
  Pack1(char mark, size_t len) : _mark(mark), _ring(len) {}

  bool pack(uvp::uv::BufT &buf, const char *p, unsigned short len) override {
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

  size_t feed(const char *data, size_t bytes) override {
    return _ring.write(data, bytes);
  }

  bool unpack(uvp::uv::BufT &buf) override {
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

  std::unique_ptr<PackInterface> clone() const override {
    return std::make_unique<Pack1>(_mark, _ring.capacity());
  }
};

class Pack2: public PackInterface {
  RingBuffer _ring;

public:
  Pack2(size_t len) : _ring(len) {}

  bool pack(uvp::uv::BufT &buf, const char *p, unsigned short len) override {
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

  size_t feed(const char *data, size_t bytes) override {
    return _ring.write(data, bytes);
  }

  bool unpack(uvp::uv::BufT &buf) override {
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

  std::unique_ptr<PackInterface> clone() const override {
    return std::make_unique<Pack2>(_ring.capacity());
  }
};

// --

} // namespace uvplus
