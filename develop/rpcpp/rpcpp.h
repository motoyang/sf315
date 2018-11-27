//
// rpcpp.h
//

#pragma once

#include <cstdint>
#include <cassert>

// --
namespace rpcpp {

#define FATAL_EXIT(err) fatal(err, __FILE__, __func__, __LINE__)
#define LOG_TRACK (LOG_INFO << " haha...")
#define LOG_FAIL_EXIT(condition, message)                                      \
  do {                                                                         \
    if (!(condition)) {                                                        \
      LOG_CRIT << message;                                                     \
      std::exit(101);                                                          \
    }                                                                          \
  } while (false)
#define LOG_FAIL_EXIT_1(condition, message, p1)                                \
  do {                                                                         \
    if (!(condition)) {                                                        \
      LOG_CRIT << message << p1;                                               \
      std::exit(101);                                                          \
    }                                                                          \
  } while (false)

// --

typedef std::uint8_t UnsignedByte;
typedef std::int8_t Byte;
typedef std::uint16_t UnsignedShort;
typedef std::int16_t Short;
typedef std::uint32_t UnsignedInt;
typedef std::int32_t Int;
typedef std::uint64_t UnsignedLong;
typedef std::int64_t Long;
typedef float Float;
typedef double Double;
typedef std::intptr_t Pointer;
typedef std::uintptr_t UnsignedPointer;

// --

using FunType = std::string (*)(Pointer, const char *, std::size_t, std::size_t,
                                std::size_t);
using ValueType = std::pair<FunType, Pointer>;

void fatal(int err, const char *fn, const char *fun, uint64_t ln);
std::ostream &hex_dump(std::ostream &o, std::string const &v);
std::string demangle(const char *name);

// --

} // namespace rpcpp

// --

namespace std {
ostream &operator<<(ostream &stream, const rpcpp::FunType &ft);
}
