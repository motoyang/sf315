#include <uvplus.hpp>

#include "cryptography.h"
#include "securelayer.h"

// --

struct SecureCenter {
  enum class RecordType : uint8_t {
    Application = 1,
    CipherChanged = 2,
    Invalidate = 0xff
  };

  constexpr static const char *HashName = "SHA-256";
  constexpr static const char *AeadName = "ChaCha20Poly1305";
  constexpr static const size_t NonceOffset = 3;

  u8vector _psk;
  ssvector _readNonce, _writeNonce;
  size_t _granularity;
  std::unique_ptr<secure::RandomNumberGenerator> _rng;
  std::unique_ptr<secure::HashFunction> _hashFun;
  std::unique_ptr<secure::Cipher_Mode> _writeCipherFun;
  std::unique_ptr<secure::Cipher_Mode> _readCipherFun;

  SecureCenter()
      : _rng(new secure::AutoSeeded_RNG),
        _hashFun(secure::HashFunction::create(HashName)),
        _writeCipherFun(
            secure::Cipher_Mode::create(AeadName, secure::ENCRYPTION)),
        _readCipherFun(
            secure::Cipher_Mode::create(AeadName, secure::DECRYPTION)) {
    _psk = secure::hex_decode("000102030405060708090a0b0c0d0e0f");
    _granularity = _writeCipherFun->update_granularity();

    reset();
  }
  void reset() {
    auto len = _writeCipherFun->default_nonce_length();
    _readNonce.assign(len, 0);
    _writeNonce.assign(len, 0);

    // read和write密码初始化
    _psk.resize(_readCipherFun->minimum_keylength());
    ssvector nonce(len, 0);
    _readCipherFun->set_key(_psk);
    _readCipherFun->start(_readNonce);
    _writeCipherFun->set_key(_psk);
    _writeCipherFun->start(_writeNonce);
  }
  void encrypt(RecordType rt, secure::secure_vector<uint8_t> &v) const {
    // 将RecordType加入尾部
    v.push_back((uint8_t)rt);

    // pading zeros
    if (v.size() <= 256) {
      uint8_t padding = _rng->next_nonzero_byte();
      v.resize(v.size() + padding);
    }

    // 对Record加密
    _writeCipherFun->finish(v);

    // 更新nonce，每次加密和解密，nonce都有更新
    ++(*(uint16_t *)(_writeNonce.data()+ NonceOffset));
    _writeCipherFun->start(_writeNonce);
  }
  void decrypt(secure::secure_vector<uint8_t> &v, bool padding) const {
    try {
      _readCipherFun->finish(v);
    } catch (secure::Exception &e) {
      std::cout << "ex: " << e.what() << std::endl;
      v.clear();
      v.push_back(0xff);
    }

    // 更新nonce，每次加密和解密，nonce都有更新
    ++(*(uint16_t *)(_readNonce.data()+ NonceOffset));
    _readCipherFun->start(_readNonce);

    if (padding) {
      // 剔除padding的zero，并取出RecordType
      auto rtAddr =
          std::find_if(v.crbegin(), v.crend(), [](uint8_t c) { return c > 0; });
      assert(rtAddr != v.crend());
      auto distance = std::distance(rtAddr, v.crend());
      v.resize(distance - 1); // -1是为了排除附加的RecordType
    }
  }
  void decrypt(RecordType &rt, secure::secure_vector<uint8_t> &v) const {
    // assert(v.size() % _granularity == 0);
    try {
      _readCipherFun->finish(v);
    } catch (secure::Exception &e) {
      std::cout << "ex: " << e.what() << std::endl;
      v.clear();
      v.push_back(0xff);
    }

    // 更新nonce，每次加密和解密，nonce都有更新
    ++(*(uint16_t *)(_readNonce.data()+ NonceOffset));
    _readCipherFun->start(_readNonce);

    // 剔除padding的zero，并取出RecordType
    auto rtAddr =
        std::find_if(v.crbegin(), v.crend(), [](uint8_t c) { return c > 0; });
    assert(rtAddr != v.crend());
    rt = (RecordType)*rtAddr;
    auto distance = std::distance(rtAddr, v.crend());
    v.resize(distance - 1); // -1是为了排除附加的RecordType
  }
  u8vector update() {
    // 生成新的write key
    _rng->randomize(_writeNonce.data(), _writeNonce.size());
    auto len = _writeCipherFun->minimum_keylength();
    ssvector wkey(len);
    _rng->randomize(wkey.data(), wkey.size());
    ssvector enkey(_writeNonce);
    enkey.insert(enkey.cend(), wkey.cbegin(), wkey.cend());

    // paddding zere，降低长度特征
    len = enkey.size();
    uint8_t paddingLen = _rng->next_nonzero_byte();
    enkey.resize(len + paddingLen);
    _rng->randomize(enkey.data() + len, paddingLen);

    // 用当前的write key对新生成的write key加密
    encrypt(RecordType::CipherChanged, enkey);

    // 更新write key和noncu
    _writeCipherFun->set_key(wkey);
    _writeCipherFun->start(_writeNonce);

    std::cout << "update wkey: " << secure::hex_encode(wkey) << std::endl
              << "nonce: " << secure::hex_encode(_writeNonce) << std::endl;

    // 将加密后的新write key发送给对方
    return u8vector(enkey.data(), enkey.data() + enkey.size());
  }
  void received(RecordType rt, const ssvector &v) {
    // 读取对方发来的write key，作为本端的read key
    auto len = _readCipherFun->default_nonce_length();
    _readNonce.assign(v.data(), v.data() + len);
    ssvector rkey(v.data() + len,
                  v.data() + len + _readCipherFun->minimum_keylength());

    std::cout << "rkey: " << secure::hex_encode(rkey) << std::endl
              << "nonce: " << secure::hex_encode(_readNonce) << std::endl;

    // 设置本端的read key
    _readCipherFun->set_key(rkey);
    ++(*(uint16_t *)(_readNonce.data()+ NonceOffset));
    _readCipherFun->start(_readNonce);
  }
};

// --

struct SecureRecordOverChunk::Impl {
  uvplus::RingBuffer _ring;
  uvplus::Chunk _chunk;
  std::unique_ptr<SecureCenter> _sc;
  uint16_t _count = 0;
  static constexpr int TagSize = 16;

  Impl(bool secure, size_t recordSize, size_t chunkSize)
      : _ring(recordSize + sizeof(SecureCenter::RecordType) +
              sizeof(Definition::_len) + TagSize),
        _chunk(chunkSize),
        _sc(secure ? std::make_unique<SecureCenter>() : nullptr) {}

  // 净负荷的长度
  size_t length() const {
    auto len = _ring.capacity() - sizeof(Definition::_len);
    if (_sc) {
      len -= (sizeof(SecureCenter::RecordType) + TagSize);
    }
    return len;
  }

  bool isExpired() {
    bool r = false;
    if (_sc && (0 == _count--)) {
      // 重置count，上下波动为256到4096+256
      _sc->_rng->randomize((uint8_t *)&_count, sizeof(_count));
      _count %= 4 * 1024;
      _count += 256;

      r = true;
    }
    return r;
  }

  bool combine(u8vector &buf) {
    int head_len = sizeof(Definition::_len);
    decltype(Definition::_len) body_len = 0;
    if (!_ring.peek((char *)&body_len, head_len)) {
      return false;
    }
    if (_ring.size() < (head_len + body_len)) {
      return false;
    }
    if (_ring.capacity() < body_len + head_len) {
      UVP_ASSERT(false);
      return false;
    }

    _ring.advance(head_len);
    if (_sc) {
      ssvector v(body_len);
      _ring.read((char *)v.data(), body_len);

      // 对收到的Record解密
      SecureCenter::RecordType rt = SecureCenter::RecordType::Invalidate;
      _sc->decrypt(rt, v);
      if (rt != SecureCenter::RecordType::Application) {
        // 如果是带外数据，就在SecureCenter中处理，不用上传到应用层
        _sc->received(rt, v);
        return false;
      }

      // 复制到buf中，作为out parameter返回
      buf.assign(v.data(), v.data() + v.size());
    } else {
      buf.resize(body_len);
      _ring.read((char *)buf.data(), body_len);
    }

    return true;
  }

  u8vlist collect(const uint8_t *p, size_t len) {
    u8vlist l;
    int remain = len;
    do {
      int writed = _ring.write((const char *)p, remain);
      remain -= writed;
      p += writed;

      while (true) {
        // 解析出每个包
        u8vector b;
        if (!combine(b)) {
          break;
        }

        l.emplace_back(std::move(b));
      }
    } while (remain > 0);

    return l;
  }

  std::list<uvp::uv::BufT> cut(const uint8_t *p, size_t len) const {
    UVP_ASSERT(len);
    UVP_ASSERT(len <=
               _ring.capacity() - sizeof(Definition::_len)); // length());

    std::list<uvp::uv::BufT> l;
    u8vector v;
    auto headLen = sizeof(Definition::_len);
    while (len > 0) {
      auto copyed = std::min(len + headLen, _chunk.length());
      v.resize(copyed);
      auto data = v.data();
      if (headLen > 0) {
        *(decltype(Definition::_len) *)data = len;
        data += headLen;
        copyed -= headLen;
        headLen = 0;
      }
      std::memcpy(data, p, copyed);
      len -= copyed;
      p += copyed;

      auto b = _chunk.pack(v.data(), v.size());
      l.emplace_back(b);
    }

    return l;
  }
};

// --

SecureRecordOverChunk::SecureRecordOverChunk(bool secure, size_t recordSize,
                                             size_t chunkSize)
    : _impl(std::make_unique<SecureRecordOverChunk::Impl>(secure, recordSize,
                                                          chunkSize)) {}

SecureRecordOverChunk::~SecureRecordOverChunk() {}

size_t SecureRecordOverChunk::length() const { return _impl->length(); }

std::list<uvp::uv::BufT> SecureRecordOverChunk::pack(const uint8_t *p,
                                                     size_t len) const {
  if (_impl->_sc) {
    ssvector v(p, p + len);
    _impl->_sc->encrypt(SecureCenter::RecordType::Application, v);
    return _impl->cut(v.data(), v.size());
  }

  return _impl->cut(p, len);
}

u8vlist SecureRecordOverChunk::feed(const char *p, size_t len) {
  u8vlist r;
  auto lv = _impl->_chunk.feed(p, len);
  for (const auto &v : lv) {
    auto lv2 = _impl->collect(v.data(), v.size());
    if (lv2.size()) {
      r.splice(r.end(), lv2);
    }
  }

  return r;
}

std::list<uvp::uv::BufT> SecureRecordOverChunk::reset() const {
  _impl->_count = 0;
  _impl->_ring.reset();
  _impl->_sc->reset();
  /*
    if (_impl->_sc) {
      _impl->_sc->reset();
      auto h = _impl->_sc->update(); // _impl->_sc->reset();
      return _impl->cut(h.data(), h.size());
    }
    */
  return std::list<uvp::uv::BufT>();
}

std::list<uvp::uv::BufT> SecureRecordOverChunk::update() const {
  if (_impl->_sc) {
    auto h = _impl->_sc->update();
    return _impl->cut(h.data(), h.size());
  }
  return std::list<uvp::uv::BufT>();
}

bool SecureRecordOverChunk::isExpired() const { return _impl->isExpired(); }

// --

struct SecureRecord::Impl {
  uvplus::RingBuffer _ring;
  std::unique_ptr<SecureCenter> _sc;
  uint16_t _count = 0;
  static constexpr int TagSize = 16;

  Impl(bool secure, size_t recordSize, size_t chunkSize)
      : _ring(recordSize + sizeof(SecureCenter::RecordType) +
              sizeof(Definition::_len) + TagSize),
        _sc(secure ? std::make_unique<SecureCenter>() : nullptr) {}

  // 净负荷的长度
  size_t length() const {
    auto len = _ring.capacity() - sizeof(Definition::_len);
    if (_sc) {
      len -= (sizeof(SecureCenter::RecordType) + TagSize);
    }
    return len;
  }

  bool isExpired() {
    bool r = false;
    if (_sc && (0 == _count--)) {
      // 重置count，上下波动为256到4096+256
      _sc->_rng->randomize((uint8_t *)&_count, sizeof(_count));
      _count %= 4 * 1024;
      _count += 256;

      r = true;
    }
    return r;
  }

  bool combine(u8vector &buf) {
    int head_len = sizeof(Definition::_len);
    decltype(Definition::_len) body_len = 0;
    if (!_ring.peek((char *)&body_len, head_len)) {
      return false;
    }
    if (_ring.size() < (head_len + body_len)) {
      return false;
    }
    if (_ring.capacity() < body_len + head_len) {
      UVP_ASSERT(false);
      return false;
    }

    _ring.advance(head_len);
    if (_sc) {
      ssvector v(body_len);
      _ring.read((char *)v.data(), body_len);

      // 对收到的Record解密
      SecureCenter::RecordType rt = SecureCenter::RecordType::Invalidate;
      _sc->decrypt(rt, v);
      if (rt != SecureCenter::RecordType::Application) {
        // 如果是带外数据，就在SecureCenter中处理，不用上传到应用层
        _sc->received(rt, v);
        return false;
      }

      // 复制到buf中，作为out parameter返回
      buf.assign(v.data(), v.data() + v.size());
    } else {
      buf.resize(body_len);
      _ring.read((char *)buf.data(), body_len);
    }

    return true;
  }

  u8vlist collect(const uint8_t *p, size_t len) {
    u8vlist l;
    int remain = len;
    do {
      int writed = _ring.write((const char *)p, remain);
      remain -= writed;
      p += writed;

      while (true) {
        // 解析出每个包
        u8vector b;
        if (!combine(b)) {
          break;
        }

        l.emplace_back(std::move(b));
      }
    } while (remain > 0);

    return l;
  }

  std::list<uvp::uv::BufT> cut(const uint8_t *p, size_t len) const {
    UVP_ASSERT(len);
    UVP_ASSERT(len <=
               _ring.capacity() - sizeof(Definition::_len)); // length());

    auto headLen = sizeof(Definition::_len);
    auto b = uvp::allocBuf(headLen + len);
    std::memcpy(b.base, &len, headLen);
    std::memcpy(b.base + headLen, p, len);

    std::list<uvp::uv::BufT> l;
    l.push_back(b);

    return l;
  }
};

// --

SecureRecord::SecureRecord(bool secure, size_t recordSize, size_t chunkSize)
    : _impl(std::make_unique<SecureRecord::Impl>(secure, recordSize,
                                                 chunkSize)) {}

SecureRecord::~SecureRecord() {}

size_t SecureRecord::length() const { return _impl->length(); }

std::list<uvp::uv::BufT> SecureRecord::pack(const uint8_t *p,
                                            size_t len) const {
  if (_impl->_sc) {
    ssvector v(p, p + len);
    _impl->_sc->encrypt(SecureCenter::RecordType::Application, v);
    return _impl->cut(v.data(), v.size());
  }

  return _impl->cut(p, len);
}

u8vlist SecureRecord::feed(const char *p, size_t len) {
  return _impl->collect((const uint8_t *)p, len);
}

std::list<uvp::uv::BufT> SecureRecord::reset() const {
  _impl->_count = 0;
  _impl->_ring.reset();
  _impl->_sc->reset();

  return std::list<uvp::uv::BufT>();
}

std::list<uvp::uv::BufT> SecureRecord::update() const {
  if (_impl->_sc) {
    auto h = _impl->_sc->update();
    return _impl->cut(h.data(), h.size());
  }
  return std::list<uvp::uv::BufT>();
}

bool SecureRecord::isExpired() const { return _impl->isExpired(); }

// --

struct SsCenter {
  constexpr static const char *HashName = "SHA-256";
  constexpr static const char *AeadName = "ChaCha20Poly1305";
  constexpr static const char *HkdfName = "HKDF(SHA-256)";
  constexpr static const char *Label = "ss-socket";
  constexpr static const size_t SaltLen = 12;
  constexpr static const size_t TagSize = 16;
  constexpr static const size_t NonceOffset = 3;

  u8vector _psk;
  ssvector _drivedKey;
  ssvector _readNonce, _writeNonce;
  std::unique_ptr<secure::RandomNumberGenerator> _rng;
  std::unique_ptr<secure::HashFunction> _hashFun;
  std::unique_ptr<secure::Cipher_Mode> _writeCipherFun, _readCipherFun;
  std::unique_ptr<secure::KDF> _hkdfFun;

  SsCenter();

  void reset();
  ssvector update();
  void encrypt(secure::secure_vector<uint8_t> &v) const;
  void decrypt(secure::secure_vector<uint8_t> &v, bool padding) const;
  bool needSalt() const;
  void salt(const ssvector &salt);
};

SsCenter::SsCenter()
    : _rng(new secure::AutoSeeded_RNG),
      _hashFun(secure::HashFunction::create(HashName)),
      _writeCipherFun(
          secure::Cipher_Mode::create(AeadName, secure::ENCRYPTION)),
      _readCipherFun(
          secure::Cipher_Mode::create(AeadName, secure::DECRYPTION)),
      _hkdfFun(secure::HKDF::create(HkdfName))     {
  _psk = secure::hex_decode("000102030405060708090a0b0c0d0e0f");
  reset();
}

void SsCenter::reset() {
  auto len = _writeCipherFun->default_nonce_length();
  _writeNonce.assign(len, 0);
  _readNonce.assign(len, 0);

  _drivedKey.clear();
}
ssvector SsCenter::update() {
  ssvector s(SaltLen);
  _rng->randomize(s.data(), s.size());
  salt(s);

  return s;
}
void SsCenter::encrypt(secure::secure_vector<uint8_t> &v) const {
  v.push_back(_rng->next_nonzero_byte());
    // pading zeros
    if (v.size() <= 256) {
      uint8_t padding = _rng->next_nonzero_byte();
      v.resize(v.size() + padding - 1);
    }

  uint32_t len = v.size() + TagSize;
  // ssvector vlen(&len, &len + sizeof(len));
  ssvector vlen(sizeof(len));
  std::memcpy(vlen.data(), &len, vlen.size());

  // 更新nonce，每次加密和解密，nonce都有更新
  ++(*(size_t *)(_writeNonce.data()+ NonceOffset));
  _writeCipherFun->start(_writeNonce);
  _writeCipherFun->finish(vlen);

  len = vlen.size();
  vlen.insert(vlen.end(), v.begin(), v.end());

  // 更新nonce，每次加密和解密，nonce都有更新
  ++(*(size_t *)(_writeNonce.data()+ NonceOffset));
  _writeCipherFun->start(_writeNonce);
  _writeCipherFun->finish(vlen, len);

  v.swap(vlen);
}
void SsCenter::decrypt(secure::secure_vector<uint8_t> &v, bool padding) const {
  // 更新nonce，每次加密和解密，nonce都有更新
  ++(*(size_t *)(_readNonce.data()+ NonceOffset));
  _readCipherFun->start(_readNonce);

  try {
    _readCipherFun->finish(v);
  } catch (secure::Exception &e) {
    std::cout << "ex: " << e.what() << std::endl;
    v.clear();
    return;
  }

  if (padding) {
    // 剔除padding的zero，并取出RecordType
    auto rtAddr =
        std::find_if(v.crbegin(), v.crend(), [](uint8_t c) { return c > 0; });
    assert(rtAddr != v.crend());
    auto distance = std::distance(rtAddr, v.crend());
    v.resize(distance - 1);
  }
}
bool SsCenter::needSalt() const { return _drivedKey.empty(); }
void SsCenter::salt(const ssvector &salt) {
  auto len = _writeCipherFun->minimum_keylength();
  _drivedKey = _hkdfFun->derive_key(len, _psk, salt,
                                 ssvector(Label, Label + std::strlen(Label)));

  std::cout << "drived key: " << secure::hex_encode(_drivedKey) << std::endl
            << "nonce: " << secure::hex_encode(_readNonce) << std::endl;

  // read和write密码初始化
  _writeCipherFun->set_key(_drivedKey);
  _readCipherFun->set_key(_drivedKey);
}

// --

struct SsRecord::Impl {
  uvplus::RingBuffer _ring;
  std::unique_ptr<SsCenter> _sc;
  uint16_t _count = 0, _body_len = 0;


  Impl(bool secure, size_t recordSize, size_t chunkSize)
      : _ring(recordSize + sizeof(SecureCenter::RecordType) +
              sizeof(Definition::_len) + SsCenter::TagSize),
        _sc(secure ? std::make_unique<SsCenter>() : nullptr) {}

  // 净负荷的长度
  size_t length() const {
    auto len = _ring.capacity() - sizeof(Definition::_len);
    if (_sc) {
      len -= (sizeof(SecureCenter::RecordType) + SsCenter::TagSize);
    }
    return len;
  }

  bool isExpired() {
    bool r = false;
    if (_sc && (0 == _count)) {
      // 重置count，上下波动为256到4096+256
      _sc->_rng->randomize((uint8_t *)&_count, sizeof(_count));
      _count %= 4 * 1024;
      _count += 256;

      r = true;
    }
    return r;
  }

  bool combine(u8vector &buf) {
    int head_len = sizeof(Definition::_len);
    ssvector v(head_len);
    if (!_ring.peek((char *)v.data(), v.size())) {
      return false;
    }
    if (_sc && _sc->needSalt()) {
      UVP_ASSERT(v.size() > SsCenter::SaltLen);
      v.resize(SsCenter::SaltLen);
      _sc->salt(v);

      _ring.advance(v.size());
      return false;
    }
    if (_sc && !_body_len) {
      _sc->decrypt(v, false);
      _body_len = *(uint16_t *)v.data();
    }
    if (_ring.size() < (head_len + _body_len)) {
      return false;
    }
    if (_ring.capacity() < _body_len + head_len) {
      UVP_ASSERT(false);
      return false;
    }

    _ring.advance(head_len);
    if (_sc) {
      v.resize(_body_len);
      _ring.read((char *)v.data(), v.size());

      // 对收到的Record解密
      _sc->decrypt(v, true);

      // 复制到buf中，作为out parameter返回
      buf.assign(v.data(), v.data() + v.size());
    } else {
      buf.resize(_body_len);
      _ring.read((char *)buf.data(), _body_len);
    }

    _body_len = 0;
    return true;
  }

  u8vlist collect(const uint8_t *p, size_t len) {
    u8vlist l;
    int remain = len;
    do {
      int writed = _ring.write((const char *)p, remain);
      remain -= writed;
      p += writed;

      while (true) {
        // 解析出每个包
        u8vector b;
        if (!combine(b)) {
          break;
        }

        l.emplace_back(std::move(b));
      }
    } while (remain > 0);

    return l;
  }

  std::list<uvp::uv::BufT> cut(const uint8_t *p, size_t len) const {
    UVP_ASSERT(len);
    UVP_ASSERT(len <= _ring.capacity());

    auto b = uvp::copyToBuf((const char *)p, len);
    std::list<uvp::uv::BufT> l;
    l.push_back(b);

    return l;
  }
};

// --

SsRecord::SsRecord(bool secure, size_t recordSize, size_t chunkSize)
    : _impl(std::make_unique<SsRecord::Impl>(secure, recordSize, chunkSize)) {}

SsRecord::~SsRecord() {}

size_t SsRecord::length() const { return _impl->length(); }

std::list<uvp::uv::BufT> SsRecord::pack(const uint8_t *p, size_t len) const {
  if (_impl->_sc) {
    ssvector v(p, p + len);
    _impl->_sc->encrypt(v);
    return _impl->cut(v.data(), v.size());
  }

  return _impl->cut(p, len);
}

u8vlist SsRecord::feed(const char *p, size_t len) {
  return _impl->collect((const uint8_t *)p, len);
}

void SsRecord::reset() const {
  _impl->_count = 0;
  _impl->_ring.reset();
  _impl->_sc->reset();
}

std::list<uvp::uv::BufT> SsRecord::update() const {
  auto s = _impl->_sc->update();
  return _impl->cut(s.data(), s.size());
}

bool SsRecord::isExpired() const { return _impl->isExpired(); }