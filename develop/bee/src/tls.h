#pragma once

#include <cstdint>
#include <cassert>
#include <netinet/in.h>

#include <type_traits>
#include <string>
#include <vector>

// --

#define NTOHS(x) ntohs(x)
#define NTOHL(x) ntohl(x)
#define HTONS(x) htons(x)
#define HTONL(x) htonl(x)

#define CONST_HTONS(x) ((uint16_t)(((x)&0xFFU) << 8 | ((x)&0xFF00U) >> 8))
#define CONST_HTONL(x)                                                         \
  (((x)&0x000000FFU) << 24 | ((x)&0x0000FF00U) << 8 | ((x)&0x00FF0000U) >> 8 | \
   ((x)&0xFF000000U) >> 24)

#define COUNT_OF(t) (sizeof(t) / sizeof(t[0]))

// --

std::string hex2section(const std::string &hex, size_t bytesOfSection = 4,
                        size_t sectionsOfLine = 4);

// --

#pragma pack(1)

using ProtocolVersion = uint16_t;
using CipherSuite = uint16_t;

constexpr ProtocolVersion PV_SSL_3_0 = CONST_HTONS(0x0300),
                          PV_TLS_1_0 = CONST_HTONS(0x0301),
                          PV_TLS_1_1 = CONST_HTONS(0x0302),
                          PV_TLS_1_2 = CONST_HTONS(0x0303),
                          PV_TLS_1_3 = CONST_HTONS(0x0304);

constexpr CipherSuite TLS_AES_128_GCM_SHA256 = CONST_HTONS(0x1301),
                      TLS_AES_256_GCM_SHA384 = CONST_HTONS(0x1302),
                      TLS_CHACHA20_POLY1305_SHA256 = CONST_HTONS(0x1303),
                      TLS_AES_128_CCM_SHA256 = CONST_HTONS(0x1304),
                      TLS_AES_128_CCM_8_SHA256 = CONST_HTONS(0x1305);

enum class ContentType : uint8_t {
  invalid = 0,
  change_cipher_spec = 20,
  alert = 21,
  handshake = 22,
  application_data = 23
};

enum class HandshakeType : uint8_t {
  client_hello = 1,
  server_hello = 2,
  new_session_ticket = 4,
  end_of_early_data = 5,
  encrypted_extensions = 8,
  certificate = 11,
  certificate_request = 13,
  certificate_verify = 15,
  finished = 20,
  key_update = 24,
  message_hash = 254
};

enum ExtensionType : uint16_t {
  // 左移8位，存储为网络字节序
  server_name = CONST_HTONS(0),                             /* RFC 6066 */
  max_fragment_length = CONST_HTONS(1),                     /* RFC 6066 */
  status_request = CONST_HTONS(5),                          /* RFC 6066 */
  supported_groups = CONST_HTONS(10),                       /* RFC 8422, 7919 */
  signature_algorithms = CONST_HTONS(13),                   /* RFC 8446 */
  use_srtp = CONST_HTONS(14),                               /* RFC 5764 */
  heartbeat = CONST_HTONS(15),                              /* RFC 6520 */
  application_layer_protocol_negotiation = CONST_HTONS(16), /* RFC 7301 */
  signed_certificate_timestamp = CONST_HTONS(18),           /* RFC 6962 */
  client_certificate_type = CONST_HTONS(19),                /* RFC 7250 */
  server_certificate_type = CONST_HTONS(20),                /* RFC 7250 */
  padding = CONST_HTONS(21),                                /* RFC 7685 */
  pre_shared_key = CONST_HTONS(41),                         /* RFC 8446 */
  early_data = CONST_HTONS(42),                             /* RFC 8446 */
  supported_versions = CONST_HTONS(43),                     /* RFC 8446 */
  cookie = CONST_HTONS(44),                                 /* RFC 8446 */
  psk_key_exchange_modes = CONST_HTONS(45),                 /* RFC 8446 */
  certificate_authorities = CONST_HTONS(47),                /* RFC 8446 */
  oid_filters = CONST_HTONS(48),                            /* RFC 8446 */
  post_handshake_auth = CONST_HTONS(49),                    /* RFC 8446 */
  signature_algorithms_cert = CONST_HTONS(50),              /* RFC 8446 */
  key_share = CONST_HTONS(51),                              /* RFC 8446 */
  // (65535)
};

enum SignatureScheme : uint16_t {
  /* RSASSA-PKCS1-v1_5 algorithms */
  rsa_pkcs1_sha256 = CONST_HTONS(0x0401),
  rsa_pkcs1_sha384 = CONST_HTONS(0x0501),
  rsa_pkcs1_sha512 = CONST_HTONS(0x0601),

  /* ECDSA algorithms */
  ecdsa_secp256r1_sha256 = CONST_HTONS(0x0403),
  ecdsa_secp384r1_sha384 = CONST_HTONS(0x0503),
  ecdsa_secp521r1_sha512 = CONST_HTONS(0x0603),

  /* RSASSA-PSS algorithms with public key OID rsaEncryption */
  rsa_pss_rsae_sha256 = CONST_HTONS(0x0804),
  rsa_pss_rsae_sha384 = CONST_HTONS(0x0805),
  rsa_pss_rsae_sha512 = CONST_HTONS(0x0806),

  /* EdDSA algorithms */
  ed25519 = CONST_HTONS(0x0807),
  ed448 = CONST_HTONS(0x0808),

  /* RSASSA-PSS algorithms with public key OID RSASSA-PSS */
  rsa_pss_pss_sha256 = CONST_HTONS(0x0809),
  rsa_pss_pss_sha384 = CONST_HTONS(0x080a),
  rsa_pss_pss_sha512 = CONST_HTONS(0x080b),

  /* Legacy algorithms */
  rsa_pkcs1_sha1 = CONST_HTONS(0x0201),
  ecdsa_sha1 = CONST_HTONS(0x0203),

  /* Reserved Code Points */
  // private_use(0xFE00..0xFFFF),
  // (0xFFFF)
};

enum NamedGroup : uint16_t {
  /* Elliptic Curve Groups (ECDHE) */
  secp256r1 = CONST_HTONS(0x0017),
  secp384r1 = CONST_HTONS(0x0018),
  secp521r1 = CONST_HTONS(0x0019),
  x25519 = CONST_HTONS(0x001D),
  x448 = CONST_HTONS(0x001E),

  /* Finite Field Groups (DHE) */
  ffdhe2048 = CONST_HTONS(0x0100),
  ffdhe3072 = CONST_HTONS(0x0101),
  ffdhe4096 = CONST_HTONS(0x0102),
  ffdhe6144 = CONST_HTONS(0x0103),
  ffdhe8192 = CONST_HTONS(0x0104),

  /* Reserved Code Points */
  // ffdhe_private_use(0x01FC..0x01FF),
  // ecdhe_private_use(0xFE00..0xFEFF),
  // (0xFFFF)
};

enum PskKeyExchangeMode : uint8_t {
  psk_ke = 0,
  psk_dhe_ke = 1
  // , (255)
};

enum AlertLevel : uint8_t {
  warning = 1,
  fatal = 2,
  // (255)
};

enum AlertDescription : uint8_t {
  close_notify = 0,
  unexpected_message = 10,
  bad_record_mac = 20,
  record_overflow = 22,
  handshake_failure = 40,
  bad_certificate = 42,
  unsupported_certificate = 43,
  certificate_revoked = 44,
  certificate_expired = 45,
  certificate_unknown = 46,
  illegal_parameter = 47,
  unknown_ca = 48,
  access_denied = 49,
  decode_error = 50,
  decrypt_error = 51,
  protocol_version = 70,
  insufficient_security = 71,
  internal_error = 80,
  inappropriate_fallback = 86,
  user_canceled = 90,
  missing_extension = 109,
  unsupported_extension = 110,
  unrecognized_name = 112,
  bad_certificate_status_response = 113,
  unknown_psk_identity = 115,
  certificate_required = 116,
  no_application_protocol = 120,
  // (255)
};

// --

// --

template <typename L, typename T> class Container {
  L _len;

public:
  L len() const {
    if constexpr (sizeof(L) == 1)
      return _len;
    else if constexpr (sizeof(L) == 2)
      return NTOHS(_len);
    else if constexpr (sizeof(L) == 4)
      return NTOHL(_len);
  }
  void len(L l) {
    if constexpr (sizeof(L) == 1)
      _len = l;
    else if constexpr (sizeof(L) == 2)
      _len = HTONS(l);
    else if constexpr (sizeof(L) == 4)
      _len = HTONL(l);
  }
  constexpr T *data() const { return (T *)(this + 1); }
  L size() const { return len() + sizeof(*this); }
  uint8_t *next() const { return (uint8_t *)(this) + size(); }
};

// --

// struct {
//   ContentType type;
//   ProtocolVersion legacy_record_version;
//   uint16 length;
//   opaque fragment[TLSPlaintext.length];
// } TLSPlaintext;
// struct {
//   opaque content[TLSPlaintext.length];
//   ContentType type;
//   uint8 zeros[length_of_padding];
// } TLSInnerPlaintext;
struct TLSPlaintext {
  ContentType type;
  ProtocolVersion legacy_record_version;

private:
  uint16_t _length;

public:
  uint16_t length() const { return NTOHS(_length); }
  void length(uint16_t len) { _length = HTONS(len); }
  constexpr uint8_t *fragment() const { return (uint8_t *)(this + 1); }
  constexpr uint8_t *innerPlaintext() const { return fragment(); }
  ContentType innerType(ContentType ct) {
    return *(ContentType *)(innerPlaintext() + length()) = ct;
  }
  uint8_t *innerZeros() const {
    return innerPlaintext() + length() + sizeof(ContentType);
  }
  uint16_t size() const { return sizeof(*this) + length(); }
  bool cryptoFlag() const { return type == ContentType::application_data; }

  static TLSPlaintext *alloc(ContentType ct, ProtocolVersion pv,
                             const uint8_t *data, uint16_t len);
  static std::vector<uint8_t> alloc(ContentType ct, ProtocolVersion pv,
                                    const uint8_t *data, uint16_t len,
                                    uint16_t length_of_padding);
};

// struct {
//   ContentType opaque_type = application_data;     /* 23 */
//   ProtocolVersion legacy_record_version = 0x0303; /* TLS v1.2 */
//   uint16 length;
//   opaque encrypted_record[TLSCiphertext.length];
// } TLSCiphertext;
struct TLSCiphertext {
  ContentType opaque_type = ContentType::application_data;     // 23
  ProtocolVersion legacy_record_version = CONST_HTONS(0x0303); // TLS v1.2
  uint16_t length;
  constexpr uint8_t *encrypted_record() const { return (uint8_t *)(this + 1); }
  uint16_t size() const { return sizeof(*this) + NTOHS(length); }

  static TLSCiphertext *alloc(const uint8_t *data, uint16_t len);
};

// --

// struct {
//   HandshakeType msg_type; /* handshake type */
//   uint24 length;          /* remaining bytes in message */
//   select(Handshake.msg_type) {
//   case client_hello:
//     ClientHello;
//   case server_hello:
//     ServerHello;
//   case end_of_early_data:
//     EndOfEarlyData;
//   case encrypted_extensions:
//     EncryptedExtensions;
//   case certificate_request:
//     CertificateRequest;
//   case certificate:
//     Certificate;
//   case certificate_verify:
//     CertificateVerify;
//   case finished:
//     Finished;
//   case new_session_ticket:
//     NewSessionTicket;
//   case key_update:
//     KeyUpdate;
//   };
// } Handshake;

class Handshake {
  uint8_t _head[4];

public:
  HandshakeType msg_type() const { return *(HandshakeType *)_head; }
  void msg_type(HandshakeType t) { _head[0] = (uint8_t)t; }
  uint32_t length() const {
    uint32_t r = 0;
    uint8_t *p = (uint8_t *)&r;
    p[0] = _head[3];
    p[1] = _head[2];
    p[2] = _head[1];
    return r;
  }
  void length(uint32_t len) {
    assert(len <= 0x00FFFFFF);
    uint8_t *p = (uint8_t *)&len;
    _head[1] = p[2];
    _head[2] = p[1];
    _head[3] = p[0];
  }
  uint8_t *message() const { return (uint8_t *)(this + 1); }
  uint32_t size() const { return sizeof(*this) + length(); }
};

// struct {
//   ExtensionType extension_type;
//   opaque extension_data<0..2 ^ 16 - 1>;
// } Extension;
template <typename T> struct Extension {
  ExtensionType extension_type;
  T *extension_data() const { return (T *)(this + 1); }
  uint32_t size() const {
    if constexpr (std::is_same_v<T, ProtocolVersion>)
      return sizeof(*this) + sizeof(T);
    else
      return sizeof(*this) + extension_data()->size();
  }
  uint8_t *next() const { return (uint8_t *)this + size(); }
};
using Extensions = Container<uint16_t, Extension<uint8_t>>;

/*
template <> struct Extension<ProtocolVersion> {
  ExtensionType extension_type;
  ProtocolVersion *extension_data() const {
    return (ProtocolVersion *)(this + 1);
  }
  uint32_t size() const { return sizeof(*this) + sizeof(ProtocolVersion); }
  uint8_t *next() const { return (uint8_t *)this + size(); }
};
*/
// using ProtocolVersion = uint16_t;
// using Random = uint8_t[32];
// uint8 CipherSuite[2]; /* Cryptographic suite selector */
// uint16 ProtocolVersion;
// opaque Random[32];
// struct {
//   ProtocolVersion legacy_version = 0x0303; /* TLS v1.2 */
//   Random random;
//   opaque legacy_session_id<0..32>;
//   CipherSuite cipher_suites<2..2 ^ 16 - 2>;
//   opaque legacy_compression_methods<1..2 ^ 8 - 1>;
//   Extension extensions<8..2 ^ 16 - 1>;
// } ClientHello;
struct ClientHello {
  ProtocolVersion legacy_version = 0x0303; /* TLS v1.2 */
  uint8_t random[32];
  auto legacy_session_id() const {
    return (Container<uint8_t, uint8_t> *)(this + 1);
  }
  auto cipher_suites() const {
    return (Container<uint16_t, CipherSuite> *)legacy_session_id()->next();
  };
  auto legacy_compression_methods() const {
    return (Container<uint8_t, uint8_t> *)cipher_suites()->next();
  }
  auto extensions() const {
    return (Extensions *)legacy_compression_methods()->next();
  }
  uint32_t size() const {
    return sizeof(*this) + legacy_session_id()->size() +
           cipher_suites()->size() + legacy_compression_methods()->size() +
           extensions()->size();
  }
};

// struct {
//   ProtocolVersion legacy_version = 0x0303; /* TLS v1.2 */
//   Random random;
//   opaque legacy_session_id_echo<0..32>;
//   CipherSuite cipher_suite;
//   uint8 legacy_compression_method = 0;
//   Extension extensions<6..2 ^ 16 - 1>;
// } ServerHello;
struct ServerHello {
  ProtocolVersion legacy_version = 0x0303; /* TLS v1.2 */
  uint8_t random[32];
  auto legacy_session_id_echo() const {
    return (Container<uint8_t, uint8_t> *)(this + 1);
  }
  auto cipher_suite() const {
    return (CipherSuite *)legacy_session_id_echo()->next();
  };
  auto legacy_compression_method() const {
    return legacy_session_id_echo()->next() + sizeof(CipherSuite);
  }
  auto extensions() const {
    return (Extensions *)(legacy_session_id_echo()->next() +
                          sizeof(CipherSuite) + sizeof(uint8_t));
  }
  uint32_t size() const {
    return sizeof(*this) + legacy_session_id_echo()->size() +
           sizeof(CipherSuite) + sizeof(uint8_t) + extensions()->size();
  }
  void helloRetryRequest();
  bool isHelloRetryRequest() const;
};

// --

// struct {
//   select(Handshake.msg_type) {
//   case client_hello:
//     ProtocolVersion versions<2..254>;
//   case server_hello: /* and HelloRetryRequest */
//     ProtocolVersion selected_version;
//   };
// } SupportedVersions;
using ClientSupportedVersions = Container<uint8_t, ProtocolVersion>;
using ServerSupportedVersion = ProtocolVersion;

// struct {
//   opaque cookie<1..2 ^ 16 - 1>;
// } Cookie;
using Cookie = Container<uint16_t, uint8_t>;

// struct {
//   SignatureScheme supported_signature_algorithms<2..2 ^ 16 - 2>;
// } SignatureSchemeList;
using SignatureSchemeList = Container<uint16_t, SignatureScheme>;

// opaque DistinguishedName<1..2 ^ 16 - 1>;
// struct {
//   DistinguishedName authorities<3..2 ^ 16 - 1>;
// } CertificateAuthoritiesExtension;
using DistinguishedName = Container<uint16_t, uint8_t>;
using CertificateAuthoritiesExtension = Container<uint16_t, DistinguishedName>;

// struct {
//   opaque certificate_extension_oid<1..2 ^ 8 - 1>;
//   opaque certificate_extension_values<0..2 ^ 16 - 1>;
// } OIDFilter;
// struct {
//   OIDFilter filters<0..2 ^ 16 - 1>;
// } OIDFilterExtension;
struct OIDFilter {
  auto certificate_extension_oid() const {
    return (Container<uint8_t, uint8_t> *)this;
  }
  auto certificate_extension_values() const {
    return (Container<uint16_t, uint8_t> *)certificate_extension_oid()->next();
  }
  uint32_t size() const {
    return certificate_extension_oid()->size() +
           certificate_extension_values()->size();
  }
};

using OIDFilterExtension = Container<uint16_t, OIDFilter>;

// struct {} PostHandshakeAuth;
struct PostHandshakeAuth {};

// struct {
//   NamedGroup named_group_list<2..2 ^ 16 - 1>;
// } NamedGroupList;
using NamedGroupList = Container<uint16_t, NamedGroup>;

// struct {
//   NamedGroup group;
//   opaque key_exchange<1..2 ^ 16 - 1>;
// } KeyShareEntry;
struct KeyShareEntry {
  NamedGroup group;
  auto key_exchange() const {
    return (Container<uint16_t, uint8_t> *)(this + 1);
  }
  uint32_t size() const { return sizeof(*this) + key_exchange()->size(); }
  auto next() const { return (KeyShareEntry *)((uint8_t *)this + size()); }
};

// struct {
//   KeyShareEntry client_shares<0..2 ^ 16 - 1>;
// } KeyShareClientHello;
using KeyShareClientHello = Container<uint16_t, KeyShareEntry>;

// struct {
//   NamedGroup selected_group;
// } KeyShareHelloRetryRequest;
struct KeyShareHelloRetryRequest {
  NamedGroup selected_group;
};

// struct {
//   KeyShareEntry server_share;
// } KeyShareServerHello;
using KeyShareServerHello = KeyShareEntry;
// struct KeyShareServerHello {
//   KeyShareEntry server_share;
//   uint16_t size() const { return sizeof(*this) + server_share.size(); }
// };

// struct {
//   PskKeyExchangeMode ke_modes<1..255>;
// } PskKeyExchangeModes;
using PskKeyExchangeModes = Container<uint8_t, PskKeyExchangeMode>;

// struct {
//   AlertLevel level;
//   AlertDescription description;
// } Alert;
struct Alert {
  AlertLevel level;
  AlertDescription description;
};

#pragma pack()

// --
