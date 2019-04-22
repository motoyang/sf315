#include <iostream>
#include <cstring>

#include <details/error.hpp>

#include "doctest/extend.h"

TEST_SUITE("details_error.hpp") {

  TEST_CASE("translateSysError") {
    CHECK(uvp::Error::translateSysError(UV_EACCES) == 0 - EACCES);
    CHECK(uvp::Error::translateSysError(UV_E2BIG) == 0 - E2BIG);
  }

  TEST_CASE("strerror") {
    DEFINE_SUBCASE_NO(sc_no);

    SUBCASE(sc_no.c_str()) {
      uvp::Error e(UV_E2BIG);
      CHECK(e.strerror() == "argument list too long");
    }

    SUBCASE(sc_no.c_str()) {
      uvp::Error e(UV_EADDRINUSE);
      CHECK(e.strerror() == "address already in use");
    }
  }

  TEST_CASE("name") {
    DEFINE_SUBCASE_NO(sc_no);

    SUBCASE(sc_no.c_str()) {
      uvp::Error e(UV_E2BIG);
      CHECK(e.name() == "E2BIG");
      std::cout << e.name() << ": " << e.name().length() << std::endl;
      std::cout << "E2BIG" << ": " << std::strlen("E2BIG") << std::endl;
    }

    SUBCASE(sc_no.c_str()) {
      uvp::Error e(UV_EADDRINUSE);
      CHECK(e.name() == "EADDRINUSE");
    }
  }
}
