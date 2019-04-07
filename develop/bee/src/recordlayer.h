#pragma once

#include <list>
#include <vector>
#include <memory>

#include "tls.h"

// --

class RecordLayer {
  constexpr static uint16_t PlaintextMaxLength = 0xFF; //0x3FFF;
  constexpr static ProtocolVersion PV = 0x0303;
  constexpr static bool ApplicationDataPadding = false;

  std::unique_ptr<secure::RandomNumberGenerator> _rng;
  std::unique_ptr<PackInterface> _packInterface;

public:
  RecordLayer();
  virtual ~RecordLayer() = default;

  std::list<TLSPlaintext *> fragment(ContentType ct, const uint8_t *data,
                                     uint32_t len) const;
  std::list<std::vector<uint8_t>>
  fragmentWithPadding(ContentType ct, const uint8_t *data, uint32_t len) const;

  std::list<std::vector<uint8_t>> feed(const uint8_t *data, size_t len) const;
};