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
  Cryptocenter(bool client);
  virtual ~Cryptocenter();

  secure::RandomNumberGenerator *rng() const;
  bool cipherSuitInit(CipherSuite cs);
  secure::HashFunction* hashFun() const;
  bool driveKey(NamedGroup ng, const std::vector<uint8_t> publicKey);
  bool driveHkdf();
  void serverCrypto(secure::secure_vector<uint8_t> &buf) const;
  void serverDecrypto(secure::secure_vector<uint8_t> &buf) const;
  void clientCrypto(secure::secure_vector<uint8_t> &buf) const;
  void clientDecrypto(secure::secure_vector<uint8_t> &buf) const;


  bool select(CipherSuite *selected,
              const Container<uint16_t, CipherSuite> *css);
  bool support(Container<uint16_t, CipherSuite> *supported) const;

  bool select(NamedGroup *selected, const NamedGroupList *ngl) const;
  bool support(NamedGroupList *ngl) const;

  bool select(KeyShareEntry *selected, const KeyShareClientHello *ksch);
  bool support(KeyShareClientHello *ksch) const;

};