#include <cassert>

#include "memoryimpl.h"
#include "transportimpl.h"

// --

BufT allocBuf(uint16_t size) {
  auto mi = MemoryInterface::get();
  uint8_t *base = mi->alloc(size);
  BufT b{base, size};
  return b;
}

BufT copyToBuf(const uint8_t *p, uint16_t len) {
  auto mi = MemoryInterface::get();
  BufT b = allocBuf(len);
  mi->copy(b.base, p, len);
  return b;
}

BufT copyToBuf(const uint8_t *p, size_t len) {
  auto mi = MemoryInterface::get();
  BufT b = allocBuf(len);
  mi->copy(b.base, p, len);
  return b;
}

BufT moveToBuf(uint8_t *p, uint16_t len) {
  BufT b;
  b.base = p;
  b.len = len;

  return b;
}

BufT moveToBuf(uint8_t *p, size_t len) {
  BufT b;
  b.base = p;
  b.len = len;

  return b;
}

void freeBuf(BufT buf) {
  assert(buf.base);
  free(buf.base);
}

// --

bool TransportInQueue::send(uint8_t *data, uint16_t len) const {
  _sink.enqueue(moveToBuf(data, len));
}

std::vector<uint8_t> TransportInQueue::recv() const {
  BufInStack bis;
  _source->try_dequeue(bis._buf);
  return std::vector<uint8_t>(bis._buf.base, bis._buf.base + bis._buf.len);
}

void TransportInQueue::source(moodycamel::ConcurrentQueue<BufT> *s) {
  _source = s;
}

// --

std::list<std::vector<uint8_t>> TransportOnStack::_list;

bool TransportOnStack::send(uint8_t *data, uint16_t len) const {
  std::vector<uint8_t> v(data, data + len);
  _list.push_back(v);
  return true;
}

std::vector<uint8_t> TransportOnStack::recv() const {
  std::vector<uint8_t> v;
  if (_list.size() > 0) {
    v = _list.front();
    _list.pop_front();
  }
  return v;
}