#pragma once

#include <concurrentqueue.h>
#include <blockingconcurrentqueue.h>
#include <ringbuffer.hpp>

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

struct GangwayInConnector {
  moodycamel::BlockingConcurrentQueue<BufT> _upward;
  moodycamel::ConcurrentQueue<BufT> _downward;
};

// --

class CodecI {
public:
  // 包的最大长度，包括head，mark和body
  // virtual unsigned short size() const = 0;
  virtual RingBuffer& ringBuffer() const = 0;
  virtual bool encode(BufT& buf, const char *p, unsigned short len) = 0;
  virtual bool decode(BufT& buf) = 0;
};

class Codec: public CodecI {
  char _mark;
  mutable RingBuffer _ring;

public:
  Codec(char mark);

  // virtual unsigned short size() const;
  virtual RingBuffer& ringBuffer() const;
  virtual bool encode(BufT& buf, const char *p, unsigned short len);
  virtual bool decode(BufT& buf);
};

class Codec2 :public CodecI {
  mutable RingBuffer _ring;

public:
  Codec2();

  // virtual unsigned short size() const override;
  virtual RingBuffer& ringBuffer() const override;
  virtual bool encode(BufT& buf, const char *p, unsigned short len) override;
  virtual bool decode(BufT& buf) override;
};
