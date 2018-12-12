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

  BufT releaseBuf();
};

// --

struct Gangway {
  moodycamel::BlockingConcurrentQueue<Packet> _upward;
  moodycamel::ConcurrentQueue<Packet> _downward;
};

// --

class RingBuffer;
class CodecI {
public:
  // 包的最大长度，包括head，mark和body
  virtual unsigned short size() const = 0;
  virtual BufT encode(const char *p, unsigned short len) = 0;
  virtual BufT decode(RingBuffer *ringbuffer) = 0;
};

class Codec: public CodecI {
  char _mark;

public:
  Codec(char mark);

  virtual unsigned short size() const;
  virtual BufT encode(const char *p, unsigned short len);
  virtual BufT decode(RingBuffer *ringbuffer);
};

class Codec2 :public CodecI {
  char _mark;

public:
  Codec2(char mark);

  virtual unsigned short size() const override;
  virtual BufT encode(const char *p, unsigned short len) override;
  virtual BufT decode(RingBuffer *ringbuffer) override;
};