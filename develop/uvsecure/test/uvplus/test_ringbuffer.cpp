#include <iostream>
#include <sstream>
#include <memory>
#include <cstring>
#include <vector>

#include <uvp.hpp>
#include <pp/prettyprint.h>

#include <plus/ringbuffer.hpp>
#include "doctest/extend.h"

TEST_SUITE("ringbuffer.hpp") {

  TEST_CASE("RingBuffer()") {
    DEFINE_SUBCASE_NO(sc_no);

    const size_t test_capacity = 68;
    uvplus::RingBuffer rb(test_capacity);

    SUBCASE(sc_no.c_str()) {
      CHECK(rb.capacity() == test_capacity);
      CHECK(rb.size() == 0);
    }

    SUBCASE(sc_no.c_str()) {
      CHECK(rb.capacity() == test_capacity);
      CHECK(rb.size() == 0);
    }
  }

  TEST_CASE("int advance(size_t)") {
    DEFINE_SUBCASE_NO(sc_no);

    const size_t test_capacity = 12;
    const char *data = "abcdefg";
    uvplus::RingBuffer rb(test_capacity);

    size_t len = std::strlen(data);
    REQUIRE(rb.write(data, len) == len);
    REQUIRE(rb.size() == len);
    REQUIRE(rb.write(data, len) == rb.capacity() - len);
    REQUIRE(rb.size() == rb.capacity());

    std::cout << rb.toString() << std::endl;

    SUBCASE(sc_no.c_str()) {
      const size_t ad_num = 3;
      CHECK(rb.advance(rb.capacity() + 1) == -1);
      CHECK(rb.advance(ad_num) == 0);
      CHECK(rb.size() == rb.capacity() - ad_num);
      CHECK(rb.advance(rb.size()) == 0);
      CHECK(rb.size() == 0);
      CHECK(rb.advance(ad_num) == -1);
      std::cout << rb.toString() << std::endl;
    }
  }

  TEST_CASE("size_t write(const char *, size_t)") {
    DEFINE_SUBCASE_NO(sc_no);

    const size_t test_capacity = 42;
    const char *data = "abcdefghijklmnopqrstuvw";
    uvplus::RingBuffer rb(test_capacity);

    SUBCASE(sc_no.c_str()) {
      CHECK(rb.capacity() != rb.size());
      CHECK(rb.write(nullptr, rb.capacity()) == 0);
      CHECK(rb.write(data, 0) == 0);
    }

    SUBCASE(sc_no.c_str()) {
      size_t len = std::strlen(data);
      CHECK(rb.write(data, len) == len);
      CHECK(rb.size() == len);
      std::cout << rb.toString() << std::endl;

      size_t writed = rb.write(data, len);
      CHECK(writed == rb.capacity() - len);
      CHECK(rb.size() == rb.capacity());
      std::cout << rb.toString() << std::endl;

      std::vector<uint8_t> peek_buf(rb.capacity(), 0);
      CHECK(rb.peek((char *)peek_buf.data(), rb.size() + 1) == nullptr);
      CHECK(rb.peek((char *)peek_buf.data(), rb.size()) != nullptr);
      peek_buf.resize(rb.size());
      std::cout << peek_buf << std::endl;
      std::cout << rb.toString() << std::endl;

      size_t adv_no = 4;
      CHECK(rb.advance(adv_no) == 0);
      CHECK(rb.size() == rb.capacity() - adv_no);
      CHECK(rb.peek((char *)peek_buf.data(), rb.size()) != nullptr);
      peek_buf.resize(rb.size());
      std::cout << peek_buf << std::endl;
      std::cout << rb.toString() << std::endl;

      CHECK(rb.write(data + writed, len - writed) == len - writed);
      peek_buf.resize(rb.size());
      CHECK(rb.peek((char *)peek_buf.data(), rb.size()) != nullptr);
      std::cout << peek_buf << std::endl;
      std::cout << rb.toString() << std::endl;

      std::vector<char> peek_buf2(rb.capacity(), 0);
      size_t space = rb.capacity() - rb.size();
      CHECK(rb.write(data, len) == space);
      CHECK(rb.size() == rb.capacity());
      peek_buf.resize(rb.size());
      CHECK(rb.peek(peek_buf2.data(), rb.size()) != nullptr);
      std::cout << peek_buf2 << std::endl;
      std::cout << rb.toString() << std::endl;
    }
  }

  TEST_CASE("char *peek(char *data, size_t bytes) const") {
    DEFINE_SUBCASE_NO(sc_no);

    const size_t test_capacity = 35;
    const char *data = "abcdefghijklmnopqrstuvw";
    uvplus::RingBuffer rb(test_capacity);

    SUBCASE(sc_no.c_str()) {
      size_t len = std::strlen(data);
      CHECK(rb.write(data, len) == len);
      CHECK(rb.size() == len);

      std::vector<uint8_t> peek_buf(rb.capacity(), 0);
      CHECK(rb.peek((char *)peek_buf.data(), rb.size() + 1) == nullptr);
      CHECK(rb.peek((char *)peek_buf.data(), rb.size()) != nullptr);
      CHECK(rb.size() == len);

      std::cout << peek_buf << std::endl;
      std::cout << rb.toString() << std::endl;
    }
  }

  TEST_CASE("size_t read(char *data, size_t bytes)") {
    DEFINE_SUBCASE_NO(sc_no);

    const size_t test_capacity = 35;
    const char *data = "abcdefghijklmnopqrstuvw";
    uvplus::RingBuffer rb(test_capacity);

    size_t len = std::strlen(data);
    CHECK(rb.write(data, len) == len);
    CHECK(rb.size() == len);

    std::vector<uint8_t> read_buf(50, 0);

    SUBCASE(sc_no.c_str()) {
      CHECK(rb.read((char *)read_buf.data(), rb.size() + 1) == 0);
    }

    SUBCASE(sc_no.c_str()) {
      size_t sz = rb.size();
      CHECK(rb.read((char *)read_buf.data(), sz) == sz);
      CHECK(std::string((const char *)read_buf.data()).compare(data) == 0);
      std::cout << read_buf << std::endl;

      CHECK(rb.write(data, len) == len);
      std::cout << rb.toString() << std::endl;

      size_t space = rb.capacity() - rb.size();
      CHECK(rb.write(data, len) == space);
      CHECK(rb.read((char *)read_buf.data(), rb.capacity()) == rb.capacity());
      CHECK(rb.size() == 0);
      std::cout << read_buf << std::endl;
      std::cout << rb.toString() << std::endl;
    }

    SUBCASE(sc_no.c_str()) {
      size_t sz = rb.size();
      CHECK(rb.read((char *)read_buf.data(), sz) == sz);
      CHECK(std::string((const char *)read_buf.data()).compare(data) == 0);
      std::cout << read_buf << std::endl;
    }
  }
}
