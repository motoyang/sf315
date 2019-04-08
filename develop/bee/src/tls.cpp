#include "memoryinterface.h"
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

std::vector<uint8_t> TLSPlaintext::alloc(ContentType ct, ProtocolVersion pv,
                                         const uint8_t *data, uint16_t len,
                                         uint16_t length_of_padding) {
  auto mi = MemoryInterface::get();
  auto buf_len = sizeof(TLSPlaintext) + len + sizeof(ct) + length_of_padding;
  std::vector<uint8_t> buf(buf_len);
  auto r = (TLSPlaintext *)buf.data();
  r->type = ct;
  r->legacy_record_version = pv;
  r->length(len);
  mi->copy(r->fragment(), data, len);
  r->innerType(ct);
  mi->set(r->innerZeros(), 0, length_of_padding);

  return buf;
}

TLSPlaintext *TLSPlaintext::alloc(ContentType ct, ProtocolVersion pv,
                                  const uint8_t *data, uint16_t len) {
  auto mi = MemoryInterface::get();
  auto r = (TLSPlaintext *)mi->alloc(sizeof(TLSPlaintext) + len);
  r->type = ct;
  r->legacy_record_version = pv;
  r->length(len);
  mi->copy(r->fragment(), data, len);

  return r;
}

TLSCiphertext *TLSCiphertext::alloc(const uint8_t *data, uint16_t len) {
  auto mi = MemoryInterface::get();
  auto r = (TLSCiphertext *)mi->alloc(sizeof(TLSCiphertext) + len);
  r->opaque_type = ContentType::application_data;
  r->legacy_record_version = 0x0303;
  r->length = HTONS(len);
  mi->copy(r->encrypted_record(), data, len);

  return r;
}
