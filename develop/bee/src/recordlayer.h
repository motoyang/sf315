#pragma once

#include <list>
#include <vector>
#include <memory>

#include "tls.h"

// --

class RecordLayer {
  struct Impl;
  std::unique_ptr<Impl> _impl;

public:
  RecordLayer();
  virtual ~RecordLayer();

  std::list<TLSPlaintext *> fragment(ContentType ct, const uint8_t *data,
                                     uint32_t len) const;
  std::list<secure::secure_vector<uint8_t>>
  fragmentWithPadding(ContentType ct, const uint8_t *data, uint32_t len) const;

  std::list<std::vector<uint8_t>> feed(const uint8_t *data, size_t len) const;
};