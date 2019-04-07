#include <iostream>
#include <string>
#include <memory>
#include <vector>

#include <botan/hex.h>

#include "src/memoryimpl.h"
#include "src/tls.h"
#include "doctest/extend.h"
#include "src/nanolog.hpp"

// --

TEST_SUITE("RecordProtocol") {
  nlog::Logger logger(nlog::NonGuaranteedLogger(1), "./", "test");

  TEST_CASE("nlog") {
    std::string s("abcdef");
    CHECK(s.size() > 0);
    NLOG_CRIT << s;
    NLOG_WARN << s;
    NLOG_INFO << s;
  }

  TEST_CASE("hex") {
    std::string s("abcdef");
    CHECK(s.size() > 0);
    std::string h = Botan::hex_encode((uint8_t *)s.data(), s.size());
    std::cout << "hex: " << h << std::endl;
    std::cout << "text: " << Botan::hex_decode(h).data() << std::endl;
  }

  TEST_CASE("TLSPlaintext") {
    DEFINE_SUBCASE_NO(sc_no);
    auto mm = std::make_shared<MemoryManager>();
    MemoryInterface::set(mm);

    std::vector<uint8_t> s(17);
    int i = 125;
    for (auto &c : s) {
      c = i++;
    }

    SUBCASE(sc_no.c_str()) {
      uint16_t len_of_padding = 3;
      auto p = TLSPlaintext::alloc(ContentType::application_data, 0x0302,
                                   s.data(), s.size(), len_of_padding);
      CHECK(p);
      CHECK(Botan::hex_encode((uint8_t *)p, p->size()) ==
            "030017020311007D7E7F808182838485868788898A8B8C8D17000000"
            );
      CHECK(Botan::hex_encode(p->innerPlaintext(), p->innerSize()) ==
            "7D7E7F808182838485868788898A8B8C8D17000000");
    }

    SUBCASE(sc_no.c_str()) {
      uint16_t len_of_padding = 23;
      auto p = TLSPlaintext::alloc(ContentType::alert, 0x0304, s.data(),
                                   s.size(), len_of_padding);
      CHECK(p);
      CHECK(Botan::hex_encode((uint8_t *)p, p->size()) ==
            "170015040311007D7E7F808182838485868788898A8B8C8D150000000000000000"
            "000000000000000000000000000000");
      CHECK(Botan::hex_encode(p->innerPlaintext(), p->innerSize()) ==
            "7D7E7F808182838485868788898A8B8C8D15000000000000000000000000000000"
            "0000000000000000");
    }
  }

  TEST_CASE("TLSCiphertext") {
    DEFINE_SUBCASE_NO(sc_no);
    auto mm = std::make_shared<MemoryManager>();
    MemoryInterface::set(mm);

    SUBCASE(sc_no.c_str()) {
      std::vector<uint8_t> s(17);
      int i = 0;
      for (auto &c : s) {
        c = i++;
      }

      auto p = TLSCiphertext::alloc(s.data(), s.size());
      CHECK(p);
      CHECK(Botan::hex_encode((uint8_t *)p, p->size()) ==
            "1703031100000102030405060708090A0B0C0D0E0F10");
      CHECK(Botan::hex_encode(p->encrypted_record(), p->length) ==
            "000102030405060708090A0B0C0D0E0F10");
    }

    SUBCASE(sc_no.c_str()) {
      std::vector<uint8_t> s(11);
      int i = 126;
      for (auto &c : s) {
        c = i++;
      }

      auto p = TLSCiphertext::alloc(s.data(), s.size());
      CHECK(p);
      CHECK(Botan::hex_encode((uint8_t *)p, p->size()) ==
            "1703030B007E7F808182838485868788");
      CHECK(Botan::hex_encode(p->encrypted_record(), p->length) ==
            "7E7F808182838485868788");
    }
  }
}