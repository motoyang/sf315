#pragma once

#include <vector>
#include <unordered_map>

#include "tls.h"
#include "cryptography.h"
#include "factory.hpp"

// --

using DH_KeyFactory =
    Factory<secure::PK_Key_Agreement_Key, NamedGroup, const secure::BigInt &>;
using ECDH_KeyFactory =
    Factory<secure::PK_Key_Agreement_Key, NamedGroup, const std::string &>;

// --

class Cryptocenter {

  std::vector<NamedGroup> _supported_ng;
  std::unordered_map<NamedGroup, std::unique_ptr<secure::PK_Key_Agreement_Key>>
      _privateKeys;

  std::vector<uint8_t> _psk;
  secure::SymmetricKey _ecdheKey;
  secure::SecureVector<uint8_t> _keys[12];

  std::unique_ptr<secure::RandomNumberGenerator> _rng;
  std::unique_ptr<secure::HashFunction> _hashFun;
  std::unique_ptr<secure::Cipher_Mode> _cipherFun;
  std::unique_ptr<secure::KDF> _hkdfExtract;
  std::unique_ptr<secure::KDF> _hkdfExpand;

  bool factoryInit();
  bool hkdfInit(const std::string& sha_name);

public:
  Cryptocenter();
  secure::RandomNumberGenerator* rng() const;
  bool driveKey(NamedGroup ng, const std::vector<uint8_t> publicKey);
  bool cipherSuitInit(CipherSuite cs);

  secure::PK_Key_Agreement_Key *privateKey(NamedGroup ng) const;
  std::vector<uint8_t> crypto(const uint8_t *p, size_t len,
                              const std::vector<uint8_t> &key) const;
  std::vector<uint8_t> crypto(const std::vector<uint8_t> &buf,
                              const std::vector<uint8_t> &key) const;
  std::vector<uint8_t> decrypto(const uint8_t *p, size_t len,
                                const std::vector<uint8_t> &key) const;
  std::vector<uint8_t> decrypto(const std::vector<uint8_t> &buf,
                                const std::vector<uint8_t> &key) const;

  bool select(CipherSuite *selected,
              const Container<uint16_t, CipherSuite> *css);
  bool support(Container<uint16_t, CipherSuite> *supported) const;

  bool select(NamedGroup *selected, const NamedGroupList *ngl) const;
  bool support(NamedGroupList *ngl) const;

  bool select(KeyShareEntry *selected, const KeyShareClientHello *ksch);
  bool support(KeyShareClientHello *ksch) const;

  void hashUpdate(const uint8_t* p, size_t len);
  bool driveKey(const ClientHello* ch, const ServerHello* sh);
};