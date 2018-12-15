#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

/*
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
*/
// --

typedef std::intptr_t Pointer;
typedef std::uintptr_t UnsignedPointer;

using FunType = std::string (*)(Pointer, const char *, std::size_t, std::size_t,
                                std::size_t);
using ValueType = std::pair<FunType, Pointer>;

// --

using BufT = uv_buf_t;
using OsFdT = uv_os_fd_t;
using File = uv_file;
using OsSock = uv_os_sock_t;

// --

enum class BufType : int {
  BUF_NULL_TYPE = 0,
  BUF_REQ_TYPE,
  BUF_REP_TYPE,
  BUF_ECHO_TYPE,
  BUF_RESOLVE_TYPE,
  BUF_TYPE_END
};
