#pragma once

#include "packinterface.h"
#include "ringbuffer.hpp"

// --

class MessagePack : public PackInterface {
  RingBuffer _ring;

public:
  MessagePack(size_t len);

  bool pack(BufT &buf, const char *p, unsigned short len) override;
  size_t feed(const char *data, size_t bytes) override;
  bool unpack(BufT &buf) override;
  std::unique_ptr<PackInterface> clone() const override;
};

class TSLPlaintextPack : public PackInterface {
  RingBuffer _ring;

public:
  TSLPlaintextPack(size_t len);

  bool pack(BufT &buf, const char *p, unsigned short len) override;
  size_t feed(const char *data, size_t bytes) override;
  bool unpack(BufT &buf) override;
  std::unique_ptr<PackInterface> clone() const override;
};

// --
