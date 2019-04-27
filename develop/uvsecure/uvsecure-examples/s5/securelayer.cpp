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

  u8vector _psk;
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
  }
  void encrypt(RecordType rt, secure::secure_vector<uint8_t> &v) const {
    // 将RecordType加入尾部
    v.push_back((uint8_t)rt);

    // Record的size必须是granularity的整数倍，不足的在后面padding zeros。
    auto len = v.size();
    if (auto needed = len % _granularity; needed > 0) {
      auto mark = len;
      len = len + _granularity - needed;
      v.resize(len);
    }

    // 对Record加密
    _writeCipherFun->update(v);
  }
  void decrypt(RecordType &rt, secure::secure_vector<uint8_t> &v) const {
    assert(v.size() % _granularity == 0);
    _readCipherFun->update(v);

    // 剔除padding的zero，并取出RecordType
    auto rtAddr =
        std::find_if(v.crbegin(), v.crend(), [](uint8_t c) { return c > 0; });
    assert(rtAddr != v.crend());
    rt = (RecordType)*rtAddr;
    auto distance = std::distance(rtAddr, v.crend());
    v.resize(distance - 1); // -1是为了排除附加的RecordType
  }
  u8vector reset() {
    // 生成新的write key
    auto len = _writeCipherFun->default_nonce_length();
    ssvector nonce(len);
    _rng->randomize(nonce.data(), nonce.size());
    len = _writeCipherFun->minimum_keylength();
    ssvector wkey(len);
    _rng->randomize(wkey.data(), wkey.size());
    ssvector enkey(nonce);
    enkey.insert(enkey.cend(), wkey.cbegin(), wkey.cend());

    auto PadingLen = 16;
    len = enkey.size();
    enkey.resize(len + PadingLen);
    _rng->randomize(enkey.data() + len, PadingLen);

    // read和write密码初始化
    _psk.resize(_readCipherFun->minimum_keylength());
    _readCipherFun->set_key(_psk);
    _readCipherFun->start(u8vector(nonce.size(), 0));
    _writeCipherFun->set_key(_psk);
    _writeCipherFun->start(u8vector(nonce.size(), 0));

    // 使用psk对新生成的write key加密
    encrypt(RecordType::CipherChanged, enkey);

    // 设置新生成的write key作为record加密的密码
    _writeCipherFun->set_key(wkey);
    _writeCipherFun->start(nonce);

    std::cout << "wkey: " << secure::hex_encode(wkey) << std::endl
              << "nonce: " << secure::hex_encode(nonce) << std::endl;

    // 将加密后的新write key发送给对方
    return u8vector(enkey.data(), enkey.data() + enkey.size());
  }
  u8vector update() {
    // 生成新的write key
    auto len = _writeCipherFun->default_nonce_length();
    ssvector nonce(len);
    _rng->randomize(nonce.data(), nonce.size());
    len = _writeCipherFun->minimum_keylength();
    ssvector wkey(len);
    _rng->randomize(wkey.data(), wkey.size());
    ssvector enkey(nonce);
    enkey.insert(enkey.cend(), wkey.cbegin(), wkey.cend());

    // 用当前的write key对新生成的write key加密
    encrypt(RecordType::CipherChanged, enkey);

    // 更新write key和noncu
    _writeCipherFun->set_key(wkey);
    _writeCipherFun->start(nonce);

    std::cout << "update wkey: " << secure::hex_encode(wkey) << std::endl
              << "nonce: " << secure::hex_encode(nonce) << std::endl;

    // 将加密后的新write key发送给对方
    return u8vector(enkey.data(), enkey.data() + enkey.size());
  }
  void received(RecordType rt, const ssvector &v) {
    // 读取对方发来的write key，作为本端的read key
    auto len = _readCipherFun->default_nonce_length();
    ssvector nonce(v.data(), v.data() + len);
    ssvector rkey(v.data() + len,
                  v.data() + len + _readCipherFun->minimum_keylength());

    std::cout << "rkey: " << secure::hex_encode(rkey) << std::endl
              << "nonce: " << secure::hex_encode(nonce) << std::endl;

    // 设置本端的read key
    _readCipherFun->set_key(rkey);
    _readCipherFun->start(nonce);
  }
};

// --

struct SecureRecord::Impl {
  uvplus::RingBuffer _ring;
  uvplus::Chunk _chunk;
  std::unique_ptr<SecureCenter> _sc;

  Impl(bool secure, size_t recordSize, size_t chunkSize)
      : _ring(recordSize + sizeof(Definition::_len)), _chunk(chunkSize),
        _sc(secure ? std::make_unique<SecureCenter>() : nullptr) {}

  size_t length() const { return _ring.capacity() - sizeof(Definition::_len); }

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
};

// --

SecureRecord::SecureRecord(bool secure, size_t recordSize, size_t chunkSize)
    : _impl(std::make_unique<SecureRecord::Impl>(secure, recordSize,
                                                 chunkSize)) {}

SecureRecord::~SecureRecord() {}

size_t SecureRecord::length() const {
  return _impl->_ring.capacity() - sizeof(Definition::_len);
}

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

std::list<uvp::uv::BufT> SecureRecord::reset() const {
  _impl->_ring.reset();

  if (_impl->_sc) {
    auto h = _impl->_sc->reset();
    return _impl->cut(h.data(), h.size());
  }
  return std::list<uvp::uv::BufT>();
}

std::list<uvp::uv::BufT> SecureRecord::update() const {
  if (_impl->_sc) {
    auto h = _impl->_sc->update();
    return _impl->cut(h.data(), h.size());
  }
  return std::list<uvp::uv::BufT>();
}
