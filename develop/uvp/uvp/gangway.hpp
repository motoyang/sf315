#pragma once

#include <concurrentqueue.h>

// --

struct Packet {
  std::string _peer;
  BufT _buf;

  Packet() {}

  Packet(const std::string &p, BufT buf) : _peer(p), _buf(buf) {}

  ~Packet() { freeBuf(_buf); }

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
};

// --

class RingBuffer;
class Codec {
  char _mark;

public:
  Codec(char mark);

  BufT encode(const char* p, size_t len);
  BufT decode(RingBuffer* ringbuffer);
};

// --

struct Gangway {
  moodycamel::ConcurrentQueue<Packet> _upward;
  moodycamel::ConcurrentQueue<Packet> _downward;
};

// --

class Business {
  WorkT _work;
  Gangway &_gangway;
  std::string _name;
  std::atomic<bool> _running{true};

  void workCallback();
  void afterWorkCallback(int status);
  void doSomething(Packet &&p);

public:
  Business(const std::string &name, Gangway &way);
  int start(LoopT *from);
  void stop();
};
