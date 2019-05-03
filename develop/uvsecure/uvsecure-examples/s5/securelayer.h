#pragma once

#include <memory>

#include <uvp.hpp>
#include <plus/plusdef.h>

// --

class SecureRecordOverChunk {
  struct Definition {
    uint32_t _len;
    auto record() const { return (uint8_t *)(this + 1); }
  };

  struct Impl;
  std::unique_ptr<Impl> _impl;

public:
  SecureRecordOverChunk(bool secure = true, size_t recordSize = 16 * 1024 - 1,
                        size_t chunkSize = 16);
  virtual ~SecureRecordOverChunk();

  size_t length() const;
  std::list<uvp::uv::BufT> pack(const uint8_t *p, size_t len) const;
  u8vlist feed(const char *p, size_t len);
  std::list<uvp::uv::BufT> reset() const;
  std::list<uvp::uv::BufT> update() const;
  bool isExpired() const;
};

// --

class SecureRecord {
  struct Definition {
    uint32_t _len;
    auto record() const { return (uint8_t *)(this + 1); }
  };

  struct Impl;
  std::unique_ptr<Impl> _impl;

public:
  SecureRecord(bool secure = true, size_t recordSize = 16 * 1024 - 1,
               size_t chunkSize = 16);
  virtual ~SecureRecord();

  size_t length() const;
  std::list<uvp::uv::BufT> pack(const uint8_t *p, size_t len) const;
  u8vlist feed(const char *p, size_t len);
  std::list<uvp::uv::BufT> reset() const;
  std::list<uvp::uv::BufT> update() const;
  bool isExpired() const;
};

// --

class SsRecord {
  struct Definition {
    uint8_t _len[20];
    auto record() const { return (uint8_t *)(this + 1); }
  };

  struct Impl;
  std::unique_ptr<Impl> _impl;

public:
  SsRecord(const std::string &psk, const std::string &cipherMode,
           size_t capacity = 16 * 1024);
  virtual ~SsRecord();

  size_t payloadSize() const;
  std::list<uvp::uv::BufT> pack(const uint8_t *p, size_t len) const;
  u8vlist feed(const char *p, size_t len);
  void reset() const;
};