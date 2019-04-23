#pragma once

#include "plusdef.h"
#include "ringbuffer.hpp"

#include "concurrentqueue.h"
#include "blockingconcurrentqueue.h"

#include <uvp.hpp>

namespace uvplus {

struct Packet {
  std::string _peer;
  u8vector _buf;

  Packet() {}
  Packet(std::string s, u8vector &&b)
      : _peer(s), _buf(std::forward<decltype(b)>(b)) {}
  Packet(Packet &&p) { *this = std::forward<decltype(p)>(p); }
  Packet &operator=(Packet &&p) {
    _peer.swap(p._peer);
    _buf.swap(p._buf);
    return *this;
  }
  Packet(const Packet &) = delete;
  Packet &operator=(const Packet &) = delete;
};

inline constexpr size_t ChunkSize = 16;
inline constexpr size_t RecordSize = 16 * 1024;

// --

class Chunk {
  struct Definition {
    uint16_t _len;
    auto data() const { return (uint8_t *)(this + 1); }
  };

  uvplus::RingBuffer _ring;

  bool unpack(u8vector &buf) {
    int head_len = sizeof(Definition::_len);
    unsigned short body_len = 0;
    if (!_ring.peek((char *)&body_len, head_len)) {
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
    buf.resize(body_len);
    _ring.read((char *)buf.data(), body_len);

    return true;
  }

public:
  Chunk(size_t length = ChunkSize) : _ring(length + sizeof(Definition::_len)) {}

  size_t length() const { return _ring.capacity() - sizeof(Definition::_len); }

  uvp::uv::BufT pack(const uint8_t *p, size_t len) const {
    UVP_ASSERT(p && len);
    UVP_ASSERT(len <= length());

    auto head_len = sizeof(Definition::_len);
    auto buf = uvp::allocBuf(head_len + len);
    *(decltype(Definition::_len) *)buf.base = len;
    memcpy(buf.base + head_len, p, len);

    return buf;
  }

  u8vlist feed(const char *p, size_t len) {
    u8vlist l;
    int remain = len;
    do {
      int writed = _ring.write(p, remain);
      remain -= writed;
      p += writed;

      while (true) {
        // 解析出每个包
        u8vector b;
        if (!unpack(b)) {
          break;
        }

        l.emplace_back(std::move(b));
      }
    } while (remain > 0);

    return l;
  }
};

// --

class Record {
  struct Definition {
    uint32_t _len;
    auto data() const { return (uint8_t *)(this + 1); }
  };

  uvplus::RingBuffer _ring;
  size_t _chunkSize;

  bool unpack(u8vector &buf) {
    int head_len = sizeof(Definition::_len);
    decltype(Definition::_len) body_len = 0;
    if (!_ring.peek((char *)&body_len, head_len)) {
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
    buf.resize(body_len);
    _ring.read((char *)buf.data(), body_len);

    return true;
  }

public:
  Record(size_t bufSize = RecordSize, size_t chunkSize = ChunkSize)
      : _ring(bufSize + sizeof(Definition::_len)), _chunkSize(chunkSize) {}

  size_t length() const { return _ring.capacity() - sizeof(Definition::_len); }

  u8vlist slice(const uint8_t *p, size_t len) const {
    UVP_ASSERT(len && len <= length());

    u8vlist l;
    u8vector buf;
    auto headLen = sizeof(Definition::_len);
    while (len > 0) {
      auto copyed = std::min(len + headLen, _chunkSize);
      buf.resize(copyed);
      auto data = buf.data();
      if (headLen > 0) {
        *(decltype(Definition::_len) *)data = len;
        data += headLen;
        copyed -= headLen;
        headLen = 0;
      }
      std::memcpy(data, p, copyed);
      l.emplace_back(std::move(buf));
      len -= copyed;
      p += copyed;
    }

    return l;
  }

  u8vlist feed(const uint8_t *p, size_t len) {
    u8vlist l;
    int remain = len;
    do {
      int writed = _ring.write((const char *)p, remain);
      remain -= writed;
      p += writed;

      while (true) {
        // 解析出每个包
        u8vector b;
        if (!unpack(b)) {
          break;
        }

        l.emplace_back(std::move(b));
      }
    } while (remain > 0);

    return l;
  }
};

class RecordLayer {
  struct Definition {
    uint32_t _len;
    auto data() const { return (uint8_t *)(this + 1); }
  };

  uvplus::RingBuffer _ring;
  Chunk _chunk;

  bool unpack(u8vector &buf) {
    int head_len = sizeof(Definition::_len);
    decltype(Definition::_len) body_len = 0;
    if (!_ring.peek((char *)&body_len, head_len)) {
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
    buf.resize(body_len);
    _ring.read((char *)buf.data(), body_len);

    return true;
  }

  u8vlist one(const uint8_t *p, size_t len) {
    u8vlist l;
    int remain = len;
    do {
      int writed = _ring.write((const char *)p, remain);
      remain -= writed;
      p += writed;

      while (true) {
        // 解析出每个包
        u8vector b;
        if (!unpack(b)) {
          break;
        }

        l.emplace_back(std::move(b));
      }
    } while (remain > 0);

    return l;
  }

public:
  RecordLayer(size_t bufSize = RecordSize, size_t chunkSize = ChunkSize)
      : _ring(bufSize + sizeof(Definition::_len)), _chunk(chunkSize) {}

  size_t length() const { return _ring.capacity() - sizeof(Definition::_len); }

  std::list<uvp::uv::BufT> slice(const uint8_t *p, size_t len) const {
    UVP_ASSERT(len && len <= length());

    std::list<uvp::uv::BufT> l;
    u8vector v;
    auto headLen = sizeof(Definition::_len);
    while (len > 0) {
      auto copyed = std::min(len + headLen, _chunk.length());
      v.resize(copyed);
      auto data = v.data();
      if (headLen > 0) {
        *(decltype(Definition::_len) *)data = len;
        data += headLen;
        copyed -= headLen;
        headLen = 0;
      }
      std::memcpy(data, p, copyed);
      len -= copyed;
      p += copyed;

      auto b = _chunk.pack(v.data(), v.size());
      l.emplace_back(b);
    }

    return l;
  }

  u8vlist feed(const char *p, size_t len) {
    u8vlist r;
    auto lv = _chunk.feed(p, len);
    for (const auto &v : lv) {
      auto lv2 = one(v.data(), v.size());
      if (lv2.size()) {
        r.splice(r.end(), lv2);
      }
    }

    return r;
  }

  void reset() {
    _ring.reset();
  }
};

// --

struct TcpStatusInterface {
  virtual void disconnected(const std::string& peer) = 0;
  virtual void connected(const std::string& peer) = 0;
};

} // namespace uvplus