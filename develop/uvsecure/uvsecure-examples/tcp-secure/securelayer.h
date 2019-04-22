#pragma once

#include <memory>

#include <uvp.hpp>
#include <plus/plusdef.h>

// --

class SecureCenter {
protected:
  struct Impl;
  std::unique_ptr<Impl> _impl;

public:
  enum class RecordType : uint8_t {
    Application = 1,
    Secure = 2,
    Invalidate = 0xff
  };

  SecureCenter();
  virtual ~SecureCenter();

  Impl* impl() const;
};

// --

inline constexpr size_t ChunkSize = 16;
inline constexpr size_t RecordSize = 16 * 1024;

class SecureRecord {
  struct Definition {
    uint32_t _len;
    auto data() const { return (uint8_t *)(this + 1); }
  };

  struct Impl;
  std::unique_ptr<Impl> _impl;

public:
  SecureRecord(size_t bufSize = RecordSize, size_t chunkSize = ChunkSize);
  virtual ~SecureRecord();

  size_t length() const;
  std::list<uvp::uv::BufT> slice(const uint8_t *p, size_t len) const;
  u8vlist feed(const char *p, size_t len);
  std::list<uvp::uv::BufT> reset();
  std::list<uvp::uv::BufT> update();
};