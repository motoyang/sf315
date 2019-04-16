#include "memoryinterface.h"
#include "cryptography.h"
#include "tls.h"

// --

std::string hex2section(const std::string &hex, size_t bytesOfSection,
                        size_t sectionsOfLine) {
  const size_t BytesOfSection = bytesOfSection * 2;
  const size_t BytesOfLine = BytesOfSection * sectionsOfLine;
  size_t len = std::min((uint)hex.size(), 0xFFFEu * 2);
  size_t buf_len = (len / BytesOfLine + 1) * (BytesOfLine + 6 + sectionsOfLine);

  std::string buf(buf_len, 0);
  char *dest = buf.data();
  for (uint16_t i = 0; i < len; ++i) {
    if (i == 0) {
      std::sprintf(dest, "%04X: ", 0);
      dest += 6;
    } else if (i % BytesOfLine == 0) {
      std::sprintf(dest, "\n%04X: ", i / 2);
      dest += 7;
    } else if (i % BytesOfSection == 0) {
      *dest++ = ' ';
    }
    *dest++ = hex.at(i);
  }

  return buf;
}

// --

TLSCiphertext *TLSCiphertext::alloc(const uint8_t *data, uint16_t len) {
  auto mi = MemoryInterface::get();
  auto r = (TLSCiphertext *)mi->alloc(sizeof(TLSCiphertext) + len);
  r->opaque_type = ContentType::application_data;
  r->legacy_record_version = 0x0303;
  r->length(len);
  mi->copy(r->encrypted_record(), data, len);

  return r;
}

// --

constexpr uint8_t s_retryRandom[] = {
    0xCF, 0x21, 0xAD, 0x74, 0xE5, 0x9A, 0x61, 0x11, 0xBE, 0x1D, 0x8C,
    0x02, 0x1E, 0x65, 0xB8, 0x91, 0xC2, 0xA2, 0x11, 0x16, 0x7A, 0xBB,
    0x8C, 0x5E, 0x07, 0x9E, 0x09, 0xE2, 0xC8, 0xA8, 0x33, 0x9C};

void ServerHello::helloRetryRequest() {
  MemoryInterface::get()->copy(random, s_retryRandom, sizeof(s_retryRandom));
}

bool ServerHello::isHelloRetryRequest() const {
  0 == MemoryInterface::get()->compare(random, s_retryRandom,
                                       sizeof(s_retryRandom));
}