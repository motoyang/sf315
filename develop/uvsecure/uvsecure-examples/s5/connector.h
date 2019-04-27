#pragma once

#include <memory>
#include <uvp.hpp>

#include "s5define.h"

// --

class Connector {
struct Impl;
  std::unique_ptr<Impl> _impl;

public:
  Connector(uvp::Loop *loop, const struct sockaddr *dest, bool secure = true);
  virtual ~Connector();
  void secureWrite(S5Record::Type t, const std::string& from, const uint8_t* p, size_t len);
};
