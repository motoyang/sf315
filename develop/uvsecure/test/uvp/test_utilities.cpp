#include <iostream>
#include <cstring>

#include <details/utilities.hpp>

#include "doctest/extend.h"

TEST_SUITE("details_utilities.hpp") {

  TEST_CASE("demangle") {
    CHECK(uvp::demangle(typeid(uvp::demangle).name()) ==
          "std::__cxx11::basic_string<char, std::char_traits<char>, "
          "std::allocator<char> > (char const*)");
    CHECK(uvp::demangle(typeid(uvp::bin2hex).name()) ==
          "std::__cxx11::basic_string<char, std::char_traits<char>, "
          "std::allocator<char> > (unsigned char const*, unsigned long)");
    CHECK(uvp::demangle(typeid(uvp::hex2section).name()) ==
          "std::__cxx11::basic_string<char, std::char_traits<char>, "
          "std::allocator<char> > (std::__cxx11::basic_string<char, "
          "std::char_traits<char>, std::allocator<char> > const&)");
  }

  TEST_CASE("hex_dump") {
    DEFINE_SUBCASE_NO(sc_no);

    std::stringstream ss;

    SUBCASE(sc_no.c_str()) {
      const char *data = "abcdefg";
      std::string out_string("0x61 0x62 0x63 0x64 0x65 0x66 0x67 ");
      uvp::hex_dump(ss, data);
      CHECK(ss.str() == out_string);
    }

    SUBCASE(sc_no.c_str()) {
      const char *data = "1234567890";
      std::string out_string(
          "0x31 0x32 0x33 0x34 0x35 0x36 0x37 0x38 0x39 0x30 ");
      uvp::hex_dump(ss, data);
      CHECK(ss.str() == out_string);

      std::string test_string = ss.str();
      CHECK(test_string.length() == out_string.length());
      CHECK(out_string.compare(ss.str().c_str()) == 0);
    }
  }

  TEST_CASE("bin2hex and hex2section") {
    DEFINE_SUBCASE_NO(sc_no);

    SUBCASE(sc_no.c_str()) {
      const char *data = "abcdefghijk";
      std::string out_string("6162636465666768696A6B");
      std::string out = uvp::bin2hex((const uint8_t *)data, std::strlen(data));
      CHECK(out == out_string);
      CHECK(out.size() == out_string.size());

      out_string = "0000: 61626364 65666768 696A6B";
      std::string section_string = uvp::hex2section(out);
      CHECK(section_string == out_string);
      std::cout << section_string << std::endl;
    }

    SUBCASE(sc_no.c_str()) {
      int i = 99;
      std::string s(51, 0);
      for (auto &c : s) {
        c = i++;
      }
      std::string out_string(
          "636465666768696A6B6C6D6E6F707172737475767778797A7B7C7D7E7F808182838485868788898A8B8C8D8E8F909192939495");
      std::string out = uvp::bin2hex((const uint8_t *)s.data(), s.size());
      CHECK(out == out_string);

      out_string = "0000: 63646566 6768696A 6B6C6D6E 6F707172\n"
                   "0010: 73747576 7778797A 7B7C7D7E 7F808182\n"
                   "0020: 83848586 8788898A 8B8C8D8E 8F909192\n"
                   "0030: 939495";
      std::string section_string = uvp::hex2section(out);
      CHECK(section_string == out_string);
      std::cout << section_string << std::endl;
    }
  }
}
