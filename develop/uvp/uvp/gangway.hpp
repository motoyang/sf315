#pragma once

#include <concurrentqueue.h>
#include <blockingconcurrentqueue.h>
// #include <safequeue.h>

// --

struct Packet {
  std::string _peer;
  BufT _buf;

  Packet();

  Packet(const std::string &p, BufT buf);
  ~Packet();

  Packet(const Packet &) = delete;
  Packet operator=(const Packet &) = delete;

  Packet(Packet &&p);
  Packet &operator=(Packet &&p);
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
  moodycamel::BlockingConcurrentQueue<Packet> _upward;
  moodycamel::ConcurrentQueue<Packet> _downward;
};

