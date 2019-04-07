#pragma once

#include <memory>

// --

struct BufT;

struct NoCopyable {
  NoCopyable() = default;
  NoCopyable(const NoCopyable&) = delete;
  NoCopyable& operator=(const NoCopyable&) = delete;
};

struct PackInterface: NoCopyable {
  virtual ~PackInterface() = default;
  virtual bool pack(BufT &buf, const char *p, unsigned short len) = 0;
  virtual size_t feed(const char *data, size_t bytes) = 0;
  virtual bool unpack(BufT &buf) = 0;
  virtual std::unique_ptr<PackInterface> clone() const = 0;
};
