#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

#include "tls.h"

// --

class Cryptocenter {
  struct Impl;
  std::unique_ptr<Impl> _impl;

  bool hkdfInit(const std::string &sha_name);

public:
  Cryptocenter();
  virtual ~Cryptocenter();

  secure::RandomNumberGenerator *rng() const;
  bool cipherSuitInit(CipherSuite cs);
  void hashUpdate(const uint8_t *p, size_t len);
  bool driveKey(NamedGroup ng, const std::vector<uint8_t> publicKey);
  bool driveHkdf();
  void crypto(secure::secure_vector<uint8_t> &buf) const;


  bool select(CipherSuite *selected,
              const Container<uint16_t, CipherSuite> *css);
  bool support(Container<uint16_t, CipherSuite> *supported) const;

  bool select(NamedGroup *selected, const NamedGroupList *ngl) const;
  bool support(NamedGroupList *ngl) const;

  bool select(KeyShareEntry *selected, const KeyShareClientHello *ksch);
  bool support(KeyShareClientHello *ksch) const;

};