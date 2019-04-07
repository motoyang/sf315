// #include "tls.h"
// #include "cryptography.h"
// #include "factory.hpp"
#include "cryptocenter.h"

// --

Cryptocenter::Cryptocenter() {
  factoryInit();

  for (const auto &it : _dhSupported) {
    _privateKeys.insert({it.first, DH_KeyFactory::create(it.first, it.second)});
  }
  for (const auto &it : _ecdhSupported) {
    _privateKeys.insert(
        {it.first, ECDH_KeyFactory::create(it.first, it.second)});
  }
}

secure::PK_Key_Agreement_Key *Cryptocenter::privateKey(NamedGroup ng) {
  return _privateKeys.at(ng).get();
}

bool Cryptocenter::factoryInit() {
  auto create_dh = [](const secure::BigInt &p)
      -> std::unique_ptr<secure::PK_Key_Agreement_Key> {
    Botan::AutoSeeded_RNG rng;
    Botan::DL_Group domain(p, 2);
    std::string kdf = "Raw";
    // generate ECDH keys
    return std::make_unique<secure::DH_PrivateKey>(rng, domain);
  };

  auto create_ecdh = [](const std::string &n)
      -> std::unique_ptr<secure::PK_Key_Agreement_Key> {
    Botan::AutoSeeded_RNG rng;
    secure::EC_Group domain(n);
    std::string kdf = "KDF1(SHA-256)";
    // generate ECDH keys
    return std::make_unique<secure::ECDH_PrivateKey>(rng, domain);
  };

  for (const auto &it : _dhSupported) {
    DH_KeyFactory::registerId(it.first, create_dh);
  }
  for (const auto &it : _ecdhSupported) {
    ECDH_KeyFactory::registerId(it.first, create_ecdh);
  }

  return true;
}

std::vector<uint8_t>
Cryptocenter::crypto(const std::vector<uint8_t> &buf,
                     const std::vector<uint8_t> &key) const {
  return std::vector<uint8_t>();
}

std::vector<uint8_t>
Cryptocenter::decrypto(const std::vector<uint8_t> &buf,
                       const std::vector<uint8_t> &key) const {
  return std::vector<uint8_t>();
}

std::vector<uint8_t>
Cryptocenter::decrypto(const uint8_t *p, size_t len,
                       const std::vector<uint8_t> &key) const {
  return std::vector<uint8_t>();
}
