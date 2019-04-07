#include <iostream>
#include <vector>

#include "memoryimpl.h"
#include "transportimpl.h"
#include "cryptography.h"
#include "channel.h"

// --

Channel::Channel() : _ti(std::make_unique<TransportOnStack>()) {}

bool Channel::send(uint8_t *p, size_t len) const {
  // std::cout << hex2section(secure::hex_encode(p, len), 4, 8) << std::endl;
  _ti->send(p, len);

  auto mi = MemoryInterface::get();
  mi->free(p);

  return true;
}

std::vector<uint8_t> Channel::recv() const {
  return _ti->recv();
}