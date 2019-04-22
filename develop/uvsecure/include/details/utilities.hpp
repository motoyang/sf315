#pragma once

#include <cassert>
#include <cxxabi.h>
#include <cstring>
#include <string>
#include <cstdlib>

#include <nanolog.hpp>

#include "error.hpp"

// --

#define UVP_ASSERT(f) assert(f)

#define INNER_LOG_ERROR(e) uvp::log_error(e, __FILE__, __func__, __LINE__)

#define UVP_LOG_ERROR(e)                                                       \
  do {                                                                         \
    if ((e) < 0) {                                                             \
      INNER_LOG_ERROR(e);                                                      \
    }                                                                          \
  } while (false)

#define UVP_LOG_ERROR_RETURN(e)                                                \
  do {                                                                         \
    if (e) {                                                                   \
      INNER_LOG_ERROR(e);                                                      \
      return (e);                                                              \
    }                                                                          \
  } while (false)

#define UVP_LOG_ERROR_EXIT(e)                                                  \
  do {                                                                         \
    if (e) {                                                                   \
      INNER_LOG_ERROR(e);                                                      \
      assert(false);                                                           \
      std::exit(e);                                                            \
    }                                                                          \
  } while (false)

#define COUNT_OF(t) (sizeof(t) / sizeof(t[0]))

// --

namespace uvp {

inline std::ostream &hex_dump(std::ostream &o, std::string const &v) {
  std::ios::fmtflags f(o.flags());
  o << std::hex;
  for (auto c : v) {
    o << "0x" << std::setw(2) << std::setfill('0')
      << (static_cast<int>(c) & 0xff) << ' ';
  }
  o.flags(f);
  return o;
}

// 处理libuv相关的错误
inline void log_error(int err, const char *fn, const char *fun, uint64_t ln) {
  Error e(err);
  nlog::inner::is_logged(nlog::LogLevel::CRIT) &&
      nlog::inner::NanoLog() ==
          nlog::inner::NanoLogLine(nlog::LogLevel::CRIT, fn, fun, ln)
              << "err: " << err << ", name: " << e.name()
              << ", desc: " << e.strerror();
}

inline std::string demangle(const char *name) {
  size_t len = 4096;
  std::string s(len, 0);
  int status = 0;
  abi::__cxa_demangle(name, s.data(), &len, &status);
  if (status) {
    LOG_WARN << "abi::__cxa_demangle() return, status: " << status;
    s = name;
  }
  return std::string(s.c_str());
}

// --

inline std::string bin2hex(const uint8_t *data, size_t len) {
  std::string buf(len * 2 + 1, 0);
  for (size_t i = 0; i < len; ++i) {
    std::sprintf(buf.data() + i * 2, "%02X", data[i]);
  }

  return std::string(buf.c_str());
}

inline std::string hex2section(const std::string &hex) {
  const size_t ByteOfSection = 4 * 2;
  const size_t ByteOfLine = ByteOfSection * 4;
  size_t len = std::min((uint)hex.size(), 0xFFFEu * 2);
  size_t buf_len = (len / ByteOfLine + 1) * (ByteOfLine + 6 + 4);

  std::string buf(buf_len, 0);
  char *dest = buf.data();
  for (uint16_t i = 0; i < len; ++i) {
    if (i == 0) {
      std::sprintf(dest, "%04X: ", 0);
      dest += 6;
    } else if (i % ByteOfLine == 0) {
      std::sprintf(dest, "\n%04X: ", i / 2);
      dest += 7;
    } else if (i % ByteOfSection == 0) {
      *dest++ = ' ';
    }
    *dest++ = hex.at(i);
  }

  return std::string(buf.c_str());
}

} // namespace uvp
