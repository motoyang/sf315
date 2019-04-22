#pragma once

#include <sstream>

#include "doctest.h"

// --

// helper for throwing exceptions
template <typename T> int throw_if(bool in, const T &ex) {
  if (in)
#ifndef DOCTEST_CONFIG_NO_EXCEPTIONS
    throw ex;
#else  // DOCTEST_CONFIG_NO_EXCEPTIONS
    ((void)ex);
#endif // DOCTEST_CONFIG_NO_EXCEPTIONS
  return 42;
}

#define DEFINE_SUBCASE_NO(sc_no)                                               \
  std::string sc_no;                                                           \
  {                                                                            \
    static size_t sc = 0;                                                      \
    std::stringstream ss;                                                      \
    ss << "SUBCASE: " << ++sc;                                                 \
    sc_no = ss.str();                                                          \
  }

// --

namespace doctest {
namespace detail {
template <> struct StringMakerBase<false> {
  template <typename T> // if T is an enum
  static typename std::enable_if<std::is_enum<T>::value, String>::type
  convert(const T &in) {
    return toString(typename std::underlying_type<T>::type(in));
  }

  template <typename T> // if T is NOT an enum
  static typename std::enable_if<!std::is_enum<T>::value, String>::type
  convert(const T &) {
    return "{?}";
  }
};
} // namespace detail
} // namespace doctest

// --
