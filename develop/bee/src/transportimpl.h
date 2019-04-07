#pragma once

#include <string>
#include <vector>
#include <list>

#include "concurrentqueue.h"
#include "blockingconcurrentqueue.h"
#include "transportinterface.h"

struct BufT {
  uint8_t *base;
  uint16_t len;
};

BufT allocBuf(uint16_t size);
BufT copyToBuf(const uint8_t *p, size_t len);
BufT moveToBuf(uint8_t *p, size_t len);
void freeBuf(BufT buf);

// --

struct Packet {
  std::string _peer;
  BufT _buf;

  Packet() : _buf{0} {}
  Packet(const std::string &p, BufT buf) : _peer(p), _buf(buf) {}
  ~Packet() {
    if (_buf.len) {
      freeBuf(_buf);
    }
  }

  Packet(const Packet &) = delete;
  Packet operator=(const Packet &) = delete;

  Packet(Packet &&p) {
    _peer = std::move(p._peer);
    _buf = p._buf;
    p._buf.base = nullptr;
    p._buf.len = 0;
  }

  Packet &operator=(Packet &&p) {
    _peer = std::move(p._peer);
    _buf = p._buf;
    p._buf.base = nullptr;
    p._buf.len = 0;
    return *this;
  }

  BufT releaseBuf() {
    BufT b = _buf;
    _buf.base = nullptr;
    _buf.len = 0;
    return b;
  }
};

struct BufInStack {
  BufT _buf;

  BufInStack() : _buf{0} {}

  BufInStack(BufT &buf) : _buf(buf) {
    buf.base = nullptr;
    buf.len = 0;
  }

  BufInStack(BufT &&buf) : _buf(buf) {}

  ~BufInStack() { freeBuf(_buf); }
};

// --

struct Gangway {
  moodycamel::BlockingConcurrentQueue<Packet> _upward;
  moodycamel::ConcurrentQueue<Packet> _downward;
};

struct GangwayInConnector {
  moodycamel::BlockingConcurrentQueue<BufT> _upward;
  moodycamel::ConcurrentQueue<BufT> _downward;
};

// --

class QueueTransport : public TransportInterface {
  mutable moodycamel::BlockingConcurrentQueue<BufT> _queue;

public:
  bool send(uint8_t *data, uint16_t len) const override;
  std::vector<uint8_t> recv() const override;
};

class TransportOnStack : public TransportInterface {
  static std::list<std::vector<uint8_t>> _list;

public:
  bool send(uint8_t *data, uint16_t len) const override;
  std::vector<uint8_t> recv() const override;
};
