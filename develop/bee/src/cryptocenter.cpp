// #include "tls.h"
// #include "cryptography.h"
#include "memoryimpl.h"
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

constexpr static CipherSuite s_supported_ciphersuits[] = {
    CONST_HTONS(0x1301), // TLS_AES_128_GCM_SHA256
    CONST_HTONS(0x1302), // TLS_AES_256_GCM_SHA384
    CONST_HTONS(0x1303), // TLS_CHACHA20_POLY1305_SHA256
    CONST_HTONS(0x1304), // TLS_AES_128_CCM_SHA256
    CONST_HTONS(0x1305)  // TLS_AES_128_CCM_8_SHA256
};

bool Cryptocenter::select(CipherSuite &selected,
                          const Container<uint16_t, CipherSuite> *css) const {
  auto cs = css->data();
  for (auto len = 0; len < css->len(); len += sizeof(CipherSuite)) {
    auto found = std::find(std::cbegin(s_supported_ciphersuits),
                           std::cend(s_supported_ciphersuits), *cs);
    if (found != std::cend(s_supported_ciphersuits)) {
      selected = *cs;
      return true;
    }
    ++cs;
  }
  return false;
}

bool Cryptocenter::support(Container<uint16_t, CipherSuite> *supported) const {
  supported->len(sizeof(s_supported_ciphersuits));
  auto cs = supported->data();
  auto count = COUNT_OF(s_supported_ciphersuits);
  for (auto i = 0; i < count; ++i) {
    cs[i] = s_supported_ciphersuits[i];
  }
  return true;
}

bool Cryptocenter::support(NamedGroupList *ngl) const {
  ngl->len(sizeof(NamedGroup) * (_dhSupported.size() + _ecdhSupported.size()));
  auto data = ngl->data();
  for (const auto& dh: _dhSupported) {
    *data = dh.first;
    ++data;
  }
  for (const auto& ecdh: _ecdhSupported) {
    *data = ecdh.first;
    ++data;
  }

  return true;
}
bool Cryptocenter::select(NamedGroup &selected,
                          const NamedGroupList *ngl) const {
  return true;
}
