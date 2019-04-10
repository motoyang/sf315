#include <iostream>
#include "memoryimpl.h"
#include "cryptocenter.h"

// --

const std::unordered_map<NamedGroup, secure::BigInt> s_dhSupported{
    {NamedGroup::ffdhe2048,
     secure::BigInt(
         "0xFFFFFFFFFFFFFFFFADF85458A2BB4A9AAFDC5620273D3CF1D8B9C583CE2D3695A9E"
         "13641146433FBCC939DCE249B3EF97D2FE363630C75D8F681B202AEC4617AD3DF1ED5"
         "D5FD65612433F51F5F066ED0856365553DED1AF3B557135E7F57C935984F0C70E0E68"
         "B77E2A689DAF3EFE8721DF158A136ADE73530ACCA4F483A797ABC0AB182B324FB61D1"
         "08A94BB2C8E3FBB96ADAB760D7F4681D4F42A3DE394DF4AE56EDE76372BB190B07A7C"
         "8EE0A6D709E02FCE1CDF7E2ECC03404CD28342F619172FE9CE98583FF8E4F1232EEF2"
         "8183C3FE3B1B4C6FAD733BB5FCBC2EC22005C58EF1837D1683B2C6F34A26C1B2EFFA8"
         "86B423861285C97FFFFFFFFFFFFFFFF")},
    {NamedGroup::ffdhe3072,
     secure::BigInt(
         "0xFFFFFFFFFFFFFFFFADF85458A2BB4A9AAFDC5620273D3CF1D8B9C583CE2D3695A9E"
         "13641146433FBCC939DCE249B3EF97D2FE363630C75D8F681B202AEC4617AD3DF1ED5"
         "D5FD65612433F51F5F066ED0856365553DED1AF3B557135E7F57C935984F0C70E0E68"
         "B77E2A689DAF3EFE8721DF158A136ADE73530ACCA4F483A797ABC0AB182B324FB61D1"
         "08A94BB2C8E3FBB96ADAB760D7F4681D4F42A3DE394DF4AE56EDE76372BB190B07A7C"
         "8EE0A6D709E02FCE1CDF7E2ECC03404CD28342F619172FE9CE98583FF8E4F1232EEF2"
         "8183C3FE3B1B4C6FAD733BB5FCBC2EC22005C58EF1837D1683B2C6F34A26C1B2EFFA8"
         "86B4238611FCFDCDE355B3B6519035BBC34F4DEF99C023861B46FC9D6E6C9077AD91D"
         "2691F7F7EE598CB0FAC186D91CAEFE130985139270B4130C93BC437944F4FD4452E2D"
         "74DD364F2E21E71F54BFF5CAE82AB9C9DF69EE86D2BC522363A0DABC521979B0DEADA"
         "1DBF9A42D5C4484E0ABCD06BFA53DDEF3C1B20EE3FD59D7C25E41D2B66C62E37FFFFF"
         "FFFFFFFFFFF")},
    {NamedGroup::ffdhe4096,
     secure::BigInt(
         "0xFFFFFFFFFFFFFFFFADF85458A2BB4A9AAFDC5620273D3CF1D8B9C583CE2D3695A9E"
         "13641146433FBCC939DCE249B3EF97D2FE363630C75D8F681B202AEC4617AD3DF1ED5"
         "D5FD65612433F51F5F066ED0856365553DED1AF3B557135E7F57C935984F0C70E0E68"
         "B77E2A689DAF3EFE8721DF158A136ADE73530ACCA4F483A797ABC0AB182B324FB61D1"
         "08A94BB2C8E3FBB96ADAB760D7F4681D4F42A3DE394DF4AE56EDE76372BB190B07A7C"
         "8EE0A6D709E02FCE1CDF7E2ECC03404CD28342F619172FE9CE98583FF8E4F1232EEF2"
         "8183C3FE3B1B4C6FAD733BB5FCBC2EC22005C58EF1837D1683B2C6F34A26C1B2EFFA8"
         "86B4238611FCFDCDE355B3B6519035BBC34F4DEF99C023861B46FC9D6E6C9077AD91D"
         "2691F7F7EE598CB0FAC186D91CAEFE130985139270B4130C93BC437944F4FD4452E2D"
         "74DD364F2E21E71F54BFF5CAE82AB9C9DF69EE86D2BC522363A0DABC521979B0DEADA"
         "1DBF9A42D5C4484E0ABCD06BFA53DDEF3C1B20EE3FD59D7C25E41D2B669E1EF16E6F5"
         "2C3164DF4FB7930E9E4E58857B6AC7D5F42D69F6D187763CF1D5503400487F55BA57E"
         "31CC7A7135C886EFB4318AED6A1E012D9E6832A907600A918130C46DC778F971AD003"
         "8092999A333CB8B7A1A1DB93D7140003C2A4ECEA9F98D0ACC0A8291CDCEC97DCF8EC9"
         "B55A7F88A46B4DB5A851F44182E1C68A007E5E655F6AFFFFFFFFFFFFFFFF")},
    {NamedGroup::ffdhe6144,
     secure::BigInt(
         "0xFFFFFFFFFFFFFFFFADF85458A2BB4A9AAFDC5620273D3CF1D8B9C583CE2D3695A9E"
         "13641146433FBCC939DCE249B3EF97D2FE363630C75D8F681B202AEC4617AD3DF1ED5"
         "D5FD65612433F51F5F066ED0856365553DED1AF3B557135E7F57C935984F0C70E0E68"
         "B77E2A689DAF3EFE8721DF158A136ADE73530ACCA4F483A797ABC0AB182B324FB61D1"
         "08A94BB2C8E3FBB96ADAB760D7F4681D4F42A3DE394DF4AE56EDE76372BB190B07A7C"
         "8EE0A6D709E02FCE1CDF7E2ECC03404CD28342F619172FE9CE98583FF8E4F1232EEF2"
         "8183C3FE3B1B4C6FAD733BB5FCBC2EC22005C58EF1837D1683B2C6F34A26C1B2EFFA8"
         "86B4238611FCFDCDE355B3B6519035BBC34F4DEF99C023861B46FC9D6E6C9077AD91D"
         "2691F7F7EE598CB0FAC186D91CAEFE130985139270B4130C93BC437944F4FD4452E2D"
         "74DD364F2E21E71F54BFF5CAE82AB9C9DF69EE86D2BC522363A0DABC521979B0DEADA"
         "1DBF9A42D5C4484E0ABCD06BFA53DDEF3C1B20EE3FD59D7C25E41D2B669E1EF16E6F5"
         "2C3164DF4FB7930E9E4E58857B6AC7D5F42D69F6D187763CF1D5503400487F55BA57E"
         "31CC7A7135C886EFB4318AED6A1E012D9E6832A907600A918130C46DC778F971AD003"
         "8092999A333CB8B7A1A1DB93D7140003C2A4ECEA9F98D0ACC0A8291CDCEC97DCF8EC9"
         "B55A7F88A46B4DB5A851F44182E1C68A007E5E0DD9020BFD64B645036C7A4E677D2C3"
         "8532A3A23BA4442CAF53EA63BB454329B7624C8917BDD64B1C0FD4CB38E8C334C701C"
         "3ACDAD0657FCCFEC719B1F5C3E4E46041F388147FB4CFDB477A52471F7A9A96910B85"
         "5322EDB6340D8A00EF092350511E30ABEC1FFF9E3A26E7FB29F8C183023C3587E38DA"
         "0077D9B4763E4E4B94B2BBC194C6651E77CAF992EEAAC0232A281BF6B3A739C122611"
         "6820AE8DB5847A67CBEF9C9091B462D538CD72B03746AE77F5E62292C311562A84650"
         "5DC82DB854338AE49F5235C95B91178CCF2DD5CACEF403EC9D1810C6272B045B3B71F"
         "9DC6B80D63FDD4A8E9ADB1E6962A69526D43161C1A41D570D7938DAD4A40E329CD0E4"
         "0E65FFFFFFFFFFFFFFFF")},
    {NamedGroup::ffdhe8192,
     secure::BigInt(
         "0xFFFFFFFFFFFFFFFFADF85458A2BB4A9AAFDC5620273D3CF1D8B9C583CE2D3695A9E"
         "13641146433FBCC939DCE249B3EF97D2FE363630C75D8F681B202AEC4617AD3DF1ED5"
         "D5FD65612433F51F5F066ED0856365553DED1AF3B557135E7F57C935984F0C70E0E68"
         "B77E2A689DAF3EFE8721DF158A136ADE73530ACCA4F483A797ABC0AB182B324FB61D1"
         "08A94BB2C8E3FBB96ADAB760D7F4681D4F42A3DE394DF4AE56EDE76372BB190B07A7C"
         "8EE0A6D709E02FCE1CDF7E2ECC03404CD28342F619172FE9CE98583FF8E4F1232EEF2"
         "8183C3FE3B1B4C6FAD733BB5FCBC2EC22005C58EF1837D1683B2C6F34A26C1B2EFFA8"
         "86B4238611FCFDCDE355B3B6519035BBC34F4DEF99C023861B46FC9D6E6C9077AD91D"
         "2691F7F7EE598CB0FAC186D91CAEFE130985139270B4130C93BC437944F4FD4452E2D"
         "74DD364F2E21E71F54BFF5CAE82AB9C9DF69EE86D2BC522363A0DABC521979B0DEADA"
         "1DBF9A42D5C4484E0ABCD06BFA53DDEF3C1B20EE3FD59D7C25E41D2B669E1EF16E6F5"
         "2C3164DF4FB7930E9E4E58857B6AC7D5F42D69F6D187763CF1D5503400487F55BA57E"
         "31CC7A7135C886EFB4318AED6A1E012D9E6832A907600A918130C46DC778F971AD003"
         "8092999A333CB8B7A1A1DB93D7140003C2A4ECEA9F98D0ACC0A8291CDCEC97DCF8EC9"
         "B55A7F88A46B4DB5A851F44182E1C68A007E5E0DD9020BFD64B645036C7A4E677D2C3"
         "8532A3A23BA4442CAF53EA63BB454329B7624C8917BDD64B1C0FD4CB38E8C334C701C"
         "3ACDAD0657FCCFEC719B1F5C3E4E46041F388147FB4CFDB477A52471F7A9A96910B85"
         "5322EDB6340D8A00EF092350511E30ABEC1FFF9E3A26E7FB29F8C183023C3587E38DA"
         "0077D9B4763E4E4B94B2BBC194C6651E77CAF992EEAAC0232A281BF6B3A739C122611"
         "6820AE8DB5847A67CBEF9C9091B462D538CD72B03746AE77F5E62292C311562A84650"
         "5DC82DB854338AE49F5235C95B91178CCF2DD5CACEF403EC9D1810C6272B045B3B71F"
         "9DC6B80D63FDD4A8E9ADB1E6962A69526D43161C1A41D570D7938DAD4A40E329CCFF4"
         "6AAA36AD004CF600C8381E425A31D951AE64FDB23FCEC9509D43687FEB69EDD1CC5E0"
         "B8CC3BDF64B10EF86B63142A3AB8829555B2F747C932665CB2C0F1CC01BD702293888"
         "39D2AF05E454504AC78B7582822846C0BA35C35F5C59160CC046FD8251541FC68C9C8"
         "6B022BB7099876A460E7451A8A93109703FEE1C217E6C3826E52C51AA691E0E423CFC"
         "99E9E31650C1217B624816CDAD9A95F9D5B8019488D9C0A0A1FE3075A577E23183F81"
         "D4A3F2FA4571EFC8CE0BA8A4FE8B6855DFE72B0A66EDED2FBABFBE58A30FAFABE1C5D"
         "71A87E2F741EF8C1FE86FEA6BBFDE530677F0D97D11D49F7A8443D0822E506A9F4614"
         "E011E2A94838FF88CD68C8BB7C5C6424CFFFFFFFFFFFFFFFF")}};

const std::unordered_map<NamedGroup, std::string> s_ecdhSupported{
      {NamedGroup::secp256r1, "secp256r1"},
      {NamedGroup::secp384r1, "secp384r1"},
      {NamedGroup::secp521r1, "secp521r1"}/*,
      {NamedGroup::x25519, "x25519"},
      {NamedGroup::x448, "x448"}*/};

// --

Cryptocenter::Cryptocenter() : _rng(new secure::AutoSeeded_RNG) {
  factoryInit();

  for (const auto &it : s_dhSupported) {
    _supported_ng.push_back(it.first);
    _privateKeys.insert({it.first, DH_KeyFactory::create(it.first, it.second)});
  }
  for (const auto &it : s_ecdhSupported) {
    _supported_ng.push_back(it.first);
    _privateKeys.insert(
        {it.first, ECDH_KeyFactory::create(it.first, it.second)});
  }
}

secure::RandomNumberGenerator *Cryptocenter::rng() const { return _rng.get(); }

secure::PK_Key_Agreement_Key *Cryptocenter::privateKey(NamedGroup ng) const {
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

  for (const auto &it : s_dhSupported) {
    DH_KeyFactory::registerId(it.first, create_dh);
  }
  for (const auto &it : s_ecdhSupported) {
    ECDH_KeyFactory::registerId(it.first, create_ecdh);
  }

  return true;
}

bool Cryptocenter::cipherSuitInit(CipherSuite cs) {
  static const std::unordered_map<CipherSuite, std::string> cipher_names = {
      {TLS_AES_128_GCM_SHA256, "AES-128/GCM"},
      {TLS_AES_256_GCM_SHA384, "AES-256/GCM"},
      {TLS_CHACHA20_POLY1305_SHA256, "ChaCha20Poly1305"},
      {TLS_AES_128_CCM_SHA256, "AES-128/CCM"},
      {TLS_AES_128_CCM_8_SHA256, "AES-128/CCM8"}};
  static const std::unordered_map<CipherSuite, std::string> hash_names = {
      {TLS_AES_128_GCM_SHA256, "SHA-256"},
      {TLS_AES_256_GCM_SHA384, "SHA-384"},
      {TLS_CHACHA20_POLY1305_SHA256, "SHA-256"},
      {TLS_AES_128_CCM_SHA256, "SHA-256"},
      {TLS_AES_128_CCM_8_SHA256, "SHA-256"}};

  _cipherFun =
      secure::Cipher_Mode::create(cipher_names.at(cs), Botan::ENCRYPTION);
  _hashFun = secure::HashFunction::create(hash_names.at(cs));
  auto len = _hashFun->output_length();
  _psk.assign(len, 0);

  hkdfInit(hash_names.at(cs));

  return true;
}

bool Cryptocenter::hkdfInit(const std::string &sha_name) {
  constexpr const char *extract_name = "HKDF-Extract(";
  constexpr const char *expand_name = "HKDF-Expand(";

  _hkdfExtract = secure::HKDF_Extract::create(extract_name + sha_name + ")");
  _hkdfExpand = secure::HKDF_Expand::create(expand_name + sha_name + ")");
}

std::vector<uint8_t>
Cryptocenter::crypto(const std::vector<uint8_t> &buf,
                     const std::vector<uint8_t> &key) const {
  return std::vector<uint8_t>();
}

bool Cryptocenter::driveKey(NamedGroup ng,
                            const std::vector<uint8_t> publicKey) {
  constexpr const char *kdf = "Raw";
  secure::PK_Key_Agreement ecdh(*_privateKeys.at(ng), *_rng, kdf);
  auto len = _hashFun->output_length();
  _ecdheKey = ecdh.derive_key(len, publicKey);
  std::cout << "ecdhekey: " << _ecdheKey.as_string() << std::endl;
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
    TLS_AES_128_GCM_SHA256, TLS_AES_256_GCM_SHA384,
    TLS_CHACHA20_POLY1305_SHA256, TLS_AES_128_CCM_SHA256,
    TLS_AES_128_CCM_8_SHA256};

bool Cryptocenter::select(CipherSuite *selected,
                          const Container<uint16_t, CipherSuite> *css) {
  auto cs = css->data();
  auto cs_count = css->len() / sizeof(*cs);
  for (const auto &c : s_supported_ciphersuits) {
    auto found = std::find(cs, cs + cs_count, c);
    if (found != cs + cs_count) {
      *selected = c;
      cipherSuitInit(*cs);
      return true;
    }
  }

  // 如果没有找到，填充Server的首先CipherSuit。
  *selected = s_supported_ciphersuits[0];
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
  ngl->len(sizeof(NamedGroup) *
           (s_dhSupported.size() + s_ecdhSupported.size()));
  auto data = ngl->data();
  for (const auto &dh : s_dhSupported) {
    *data = dh.first;
    ++data;
  }
  for (const auto &ecdh : s_ecdhSupported) {
    *data = ecdh.first;
    ++data;
  }

  return true;
}

bool Cryptocenter::select(NamedGroup *selected,
                          const NamedGroupList *ngl) const {
  auto ng = ngl->data();
  auto ng_count = ngl->len() / sizeof(NamedGroup);
  for (const auto &sng : _supported_ng) {
    auto found = std::find(ng, ng + ng_count, sng);
    if (found != ng + ng_count) {
      *selected = sng;
      return true;
    }
  }

  *selected = _supported_ng.front();
  return false;
}

// 根据ClientHello中的KeyShareClientHello，选择NamedGroup，填充KeyShareEntry，
// 返回值为true。
// 如果不能找到合适的NamedGroup，就填充希望的NamedGroup，返回值为false，Server发
// 现返回值为false时，需要向Client发送HelloRetryRequest。
bool Cryptocenter::select(KeyShareEntry *selected,
                          const KeyShareClientHello *ksch) {
  auto kse = ksch->data();
  for (auto len = 0; len < ksch->len(); len += kse->size()) {
    auto found =
        std::find(_supported_ng.cbegin(), _supported_ng.cend(), kse->group);
    if (found != _supported_ng.cend()) {
      // 根据选择的NamedGroup，向Client返回KeySharedEntry。
      selected->group = kse->group;
      auto pk = _privateKeys.at(selected->group)->public_value();
      selected->key_exchange()->len(pk.size());
      MemoryInterface::get()->copy(selected->key_exchange()->data(), pk.data(),
                                   pk.size());

      // 根据选中的NamedGroup，计算出key
      auto data = kse->key_exchange()->data();
      auto len = kse->key_exchange()->len();
      std::vector<uint8_t> publicKey(data, data + len);
      driveKey(kse->group, publicKey);

      return true;
    }
    kse = kse->next();
  }

  // 如果没有找到匹配的KeyShareEntry，就在HelloRegryRequest中返回Server首先的NamedGroup
  selected->group = _supported_ng.front();
  return false;
}

bool Cryptocenter::support(KeyShareClientHello *ksch) const {
  auto len = 0;
  auto kse = ksch->data();

  for (const auto &ng : _supported_ng) {
    kse->group = ng;

    auto pk = _privateKeys.at(ng)->public_value();
    auto key_exch = kse->key_exchange();
    key_exch->len(pk.size());
    MemoryInterface::get()->copy((uint8_t *)key_exch->data(), pk.data(),
                                 pk.size());

    len += kse->size();
    kse = kse->next();
  }
  ksch->len(len);

  return true;
}

void Cryptocenter::hashUpdate(const uint8_t *p, size_t len) {
  _hashFun->update(p, len);
}

constexpr int IndexOfEarlySecret = 0, IndexOfBinderKey = 1,
              IndexOfClientEarlyTrafficSecret = 2,
              IndexOfEarlyExporterMasterSecret = 3, IndexOfHandshakeSecret = 4,
              IndexOfClientHandshakeTrafficSecret = 5,
              IndexOfServerHandshakeTrafficSecret = 6, IndexOfMasterSecret = 7,
              IndexOfClientApplicationTrafficSecret = 8,
              IndexOfServerApplicationTrafficSecret = 9,
              IndexOfExporterMasterSecret = 10,
              IndexOfResumptionMasterSecret = 11;

bool Cryptocenter::driveKey(const ClientHello *ch, const ServerHello *sh) {
  // _hashFun->update((uint8_t *)ch, ch->size());
  // _hashFun->update((uint8_t *)sh, sh->size());
  std::unique_ptr<secure::HashFunction> tmpHash(_hashFun->clone());
  auto hashValue = tmpHash->final();

  auto len = _hashFun->output_length();
  std::vector<uint8_t> zero(len, 0);
  std::vector<uint8_t> label;

  _keys[IndexOfEarlySecret] = _hkdfExtract->derive_key(len, _psk, zero, label);
  auto tmpKey =
      _hkdfExpand->derive_key(len, _keys[IndexOfEarlySecret], "derived", "");
  _keys[IndexOfHandshakeSecret] =
      _hkdfExtract->derive_key(len, _ecdheKey.bits_of(), tmpKey, label);
  std::string s("c hs traffic");
  std::vector<uint8_t> salt(s.cbegin(), s.cend());
  _keys[IndexOfClientHandshakeTrafficSecret] = _hkdfExpand->derive_key(
      len, _keys[IndexOfHandshakeSecret], salt, hashValue);
  s = "s hs traffic";
  salt.assign(s.cbegin(), s.cend());
  _keys[IndexOfServerHandshakeTrafficSecret] = _hkdfExpand->derive_key(
      len, _keys[IndexOfHandshakeSecret], salt, hashValue);

  std::cout << "ServerHandshakeTrafficSecret key: "
            << secure::hex_encode(_keys[IndexOfServerHandshakeTrafficSecret])
            << std::endl;
  /*
    // TBF!!!
    tmpKey = _hkdfExpand->derive_key(len, _keys[IndexOfHandshakeSecret],
                                     "derived", "");
    _keys[IndexOfMasterSecret] =
        _hkdfExtract->derive_key(len, zero, tmpKey, label);
    _keys[IndexOfClientApplicationTrafficSecret] =
        _hkdfExpand->derive_key(len, _keys[IndexOfMasterSecret],
                                secure::hex_decode("c ap traffic"), hashValue);
    _keys[IndexOfServerApplicationTrafficSecret] =
        _hkdfExpand->derive_key(len, _keys[IndexOfMasterSecret],
                                secure::hex_decode("s ap traffic"), hashValue);
    _keys[IndexOfExporterMasterSecret] =
        _hkdfExpand->derive_key(len, _keys[IndexOfMasterSecret],
                                secure::hex_decode("exp master"), hashValue);
    _keys[IndexOfResumptionMasterSecret] =
        _hkdfExpand->derive_key(len, _keys[IndexOfMasterSecret],
                                secure::hex_decode("res master"), hashValue);
  */
}
