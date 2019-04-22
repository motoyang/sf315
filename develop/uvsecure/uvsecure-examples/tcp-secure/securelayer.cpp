#include <uvplus.hpp>

#include "cryptography.h"
#include "securelayer.h"

using ssvector = secure::secure_vector<uint8_t>;
using ssvlist = std::list<ssvector>;

constexpr static const char *HashName = "SHA-256";
constexpr static const char *AeadName = "ChaCha20Poly1305";

// --

struct SecureCenter::Impl {
  u8vector _psk;
  size_t _granularity;
  std::unique_ptr<secure::RandomNumberGenerator> _rng;
  std::unique_ptr<secure::HashFunction> _hashFun;
  std::unique_ptr<secure::Cipher_Mode> _writeCipherFun;
  std::unique_ptr<secure::Cipher_Mode> _readCipherFun;

  Impl()
      : _rng(new secure::AutoSeeded_RNG),
        _hashFun(secure::HashFunction::create(HashName)),
        _writeCipherFun(
            secure::Cipher_Mode::create(AeadName, secure::ENCRYPTION)),
        _readCipherFun(
            secure::Cipher_Mode::create(AeadName, secure::DECRYPTION)) {
    _psk = secure::hex_decode("000102030405060708090a0b0c0d0e0f");
    _granularity = _writeCipherFun->update_granularity();
  }
  void encrypt(RecordType rt, secure::secure_vector<uint8_t> &v) const {
    v.push_back((uint8_t)rt);
    auto len = v.size();
    if (auto needed = len % _granularity; needed > 0) {
      auto mark = len;
      len = len + _granularity - needed;
      v.resize(len);
    }
    _writeCipherFun->update(v);
  }
  void decrypt(RecordType &rt, secure::secure_vector<uint8_t> &v) const {
    assert(v.size() % _granularity == 0);
    _readCipherFun->update(v);

    // 剔除padding的zero
    auto ct_addr =
        std::find_if(v.crbegin(), v.crend(), [](uint8_t c) { return c > 0; });
    assert(ct_addr != v.crend());
    rt = (RecordType)*ct_addr;
    auto distance = std::distance(ct_addr, v.crend());
    v.resize(distance - 1); // -1是为了排除TLSInnerPlaintext.type
  }
  u8vector hello() {
    _psk.resize(_readCipherFun->minimum_keylength());
    _readCipherFun->set_key(_psk);
    _readCipherFun->start(u8vector(_readCipherFun->default_nonce_length(), 0));

    auto len = _writeCipherFun->default_nonce_length();
    ssvector nonce(len);
    _rng->randomize(nonce.data(), nonce.size());
    len = _writeCipherFun->minimum_keylength();
    ssvector wkey(len);
    _rng->randomize(wkey.data(), wkey.size());
    ssvector enkey(nonce);
    enkey.insert(enkey.cend(), wkey.cbegin(), wkey.cend());

    // 使用psk对write key加密
    _writeCipherFun->set_key(_psk);
    _writeCipherFun->start(u8vector(nonce.size(), 0));
    encrypt(RecordType::Secure, enkey);

    // 设置write key作为record加密的密码
    _writeCipherFun->set_key(wkey);
    _writeCipherFun->start(nonce);

    std::cout << "wkey: " << secure::hex_encode(wkey) << std::endl
              << "nonce: " << secure::hex_encode(nonce) << std::endl;

    return u8vector(enkey.data(), enkey.data() + enkey.size());
  }
  u8vector update() {
    auto len = _writeCipherFun->default_nonce_length();
    ssvector nonce(len);
    _rng->randomize(nonce.data(), nonce.size());
    len = _writeCipherFun->minimum_keylength();
    ssvector wkey(len);
    _rng->randomize(wkey.data(), wkey.size());
    ssvector enkey(nonce);
    enkey.insert(enkey.cend(), wkey.cbegin(), wkey.cend());

    encrypt(RecordType::Secure, enkey);

    // 更新write key和noncu
    _writeCipherFun->set_key(wkey);
    _writeCipherFun->start(nonce);

    std::cout << "update wkey: " << secure::hex_encode(wkey) << std::endl
              << "nonce: " << secure::hex_encode(nonce) << std::endl;

    return u8vector(enkey.data(), enkey.data() + enkey.size());
  }
  void received(RecordType rt, const ssvector &v) {
    auto len = _readCipherFun->default_nonce_length();
    ssvector nonce(v.data(), v.data() + len);
    ssvector rkey(v.data() + len,
                  v.data() + len + _readCipherFun->minimum_keylength());

    std::cout << "rkey: " << secure::hex_encode(rkey) << std::endl
              << "nonce: " << secure::hex_encode(nonce) << std::endl;

    _readCipherFun->set_key(rkey);
    _readCipherFun->start(nonce);
  }
};

SecureCenter::SecureCenter() : _impl(std::make_unique<SecureCenter::Impl>()) {}

SecureCenter::~SecureCenter() {}

SecureCenter::Impl *SecureCenter::impl() const { return _impl.get(); }

// --

struct SecureRecord::Impl {
  uvplus::RingBuffer _ring;
  uvplus::Chunk _chunk;
  SecureCenter _sc;

  Impl(size_t bufSize, size_t chunkSize)
      : _ring(bufSize + sizeof(Definition::_len)), _chunk(chunkSize) {}

  size_t length() const { return _ring.capacity() - sizeof(Definition::_len); }

  bool unpack(u8vector &buf) {
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
    ssvector v(body_len);
    _ring.read((char *)v.data(), body_len);

    // 对收到的Record解密
    SecureCenter::RecordType rt = SecureCenter::RecordType::Invalidate;
    _sc.impl()->decrypt(rt, v);
    if (rt != SecureCenter::RecordType::Application) {
      // 如果是带外数据，就在SecureCenter中处理，不用上传到应用层
      _sc.impl()->received(rt, v);
      return false;
    }

    buf.assign(v.data(), v.data() + v.size());
    return true;
  }

  u8vlist one(const uint8_t *p, size_t len) {
    u8vlist l;
    int remain = len;
    do {
      int writed = _ring.write((const char *)p, remain);
      remain -= writed;
      p += writed;

      while (true) {
        // 解析出每个包
        u8vector b;
        if (!unpack(b)) {
          break;
        }

        l.emplace_back(std::move(b));
      }
    } while (remain > 0);

    return l;
  }

  std::list<uvp::uv::BufT> two(const uint8_t *p, size_t len) const {
    UVP_ASSERT(len);
    UVP_ASSERT(len <= length());

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

  std::list<uvp::uv::BufT> hello() {
    _ring.reset();

    auto h = _sc.impl()->hello();
    return two(h.data(), h.size());
  }

  std::list<uvp::uv::BufT> update() {
    auto h = _sc.impl()->update();
    return two(h.data(), h.size());
  }
};

// --

SecureRecord::SecureRecord(size_t bufSize, size_t chunkSize)
    : _impl(std::make_unique<SecureRecord::Impl>(bufSize, chunkSize)) {}

SecureRecord::~SecureRecord() {}

size_t SecureRecord::length() const {
  return _impl->_ring.capacity() - sizeof(Definition::_len);
}

std::list<uvp::uv::BufT> SecureRecord::slice(const uint8_t *p,
                                             size_t len) const {
  ssvector v(p, p + len);
  _impl->_sc.impl()->encrypt(SecureCenter::RecordType::Application, v);
  return _impl->two(v.data(), v.size());
}

u8vlist SecureRecord::feed(const char *p, size_t len) {
  u8vlist r;
  auto lv = _impl->_chunk.feed(p, len);
  for (const auto &v : lv) {
    auto lv2 = _impl->one(v.data(), v.size());
    if (lv2.size()) {
      r.splice(r.end(), lv2);
    }
  }

  return r;
}

std::list<uvp::uv::BufT> SecureRecord::reset() { return _impl->hello(); }

std::list<uvp::uv::BufT> SecureRecord::update() { return _impl->update(); }
