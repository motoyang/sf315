#include <iostream>
#include <vector>

#include "memoryimpl.h"
#include "transportimpl.h"
#include "cryptography.h"
#include "channel.h"

// --

struct Channel::Impl {
  moodycamel::ConcurrentQueue<BufT> _sink;
  moodycamel::ConcurrentQueue<BufT> *_source;
};

// --

Channel::Channel() : _impl(std::make_unique<Channel::Impl>()) {}

Channel::~Channel() = default;

bool Channel::send(const uint8_t *p, size_t len) const {
  _impl->_sink.enqueue(copyToBuf(p, len));

  return true;
}

secure::secure_vector<uint8_t> Channel::recv() const {
  BufT b;
  if (_impl->_source->try_dequeue(b)) {
    BufPtr p(&b, freeBuf2);
    return secure::secure_vector<uint8_t>(b.base, b.base + b.len);
  } else {
    return secure::secure_vector<uint8_t>();
  }
}

void Channel::bind(Channel *peer) { _impl->_source = &peer->_impl->_sink; }
