#pragma once

#include <cstdint>
#include <cstddef>

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

#pragma pack(1)

template <typename L, typename T> class Container {
  L _len;

public:
  using Head = L;
  using Data = T;

  Head len() const {
    if constexpr (sizeof(Head) == 1)
      return _len;
    else if constexpr (sizeof(Head) == 2)
      return NTOHS(_len);
    else if constexpr (sizeof(Head) == 4)
      return NTOHL(_len);
  }
  void len(Head l) {
    if constexpr (sizeof(Head) == 1)
      _len = l;
    else if constexpr (sizeof(Head) == 2)
      _len = HTONS(l);
    else if constexpr (sizeof(Head) == 4)
      _len = HTONL(l);
  }
  constexpr Data *data() const { return (Data *)(this + 1); }
  Head size() const { return len() + sizeof(*this); }
  uint8_t *next() const { return (uint8_t *)(this) + size(); }
};

struct S5Record {
  using FromType = Container<uint8_t, char>;
  using DataType = Container<uint16_t, uint8_t>;

  enum class Type : uint8_t {
    Request = 1,
    Data = 2,
    Reply = 0x10,
    Invalidate = 0xff
  };

  Type type;
  auto from() const { return (FromType *)(this + 1); }
  auto data() const {
    return (DataType *)((uint8_t *)(this + 1) + from()->size());
  }
  static constexpr size_t HeadLen() {
    return sizeof(Type) + sizeof(FromType::Head) + sizeof(DataType::Head);
  }
  size_t size() const {
    return sizeof(*this) + from()->size() + data()->size();
  }
};

// --

struct Hello {
  uint8_t ver;
  uint8_t nmethonds;
  auto methonds() const { return (uint8_t *)(this + 1); }
  auto size() const { return sizeof(*this) + nmethonds; }
};

struct Request {
  uint8_t ver;
  uint8_t cmd;
  uint8_t rsv;
  uint8_t atype;
  uint8_t len;
  auto addr() const { return (char *)(this + 1); }
  auto port() const {
    uint16_t p = *(uint16_t *)(addr() + len);
    return ntohs(p);
  }
  auto size() const { return sizeof(*this) + len + sizeof(uint16_t); }
};

// --

#pragma pack(0)
