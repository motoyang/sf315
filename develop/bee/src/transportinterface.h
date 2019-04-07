#pragma once

#include <vector>
#include "tls.h"

// --

struct TransportInterface {
  virtual bool send(uint8_t *data, uint16_t len) const = 0;
  virtual std::vector<uint8_t> recv() const = 0;

  virtual ~TransportInterface() = default;
};
