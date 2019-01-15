#pragma once

#include <cassert>
#include <cxxabi.h>

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

inline const char *demangle(const char *name) {
  char buf[1024] = {0};
  size_t len = sizeof(buf);
  int status = 0;
  abi::__cxa_demangle(name, buf, &len, &status);
  std::string s;
  if (status) {
    LOG_WARN << "abi::__cxa_demangle() return, status: " << status;
    s = name;
  } else {
    s = buf;
  }
  return s.c_str();
}

// --

} // namespace uvp
