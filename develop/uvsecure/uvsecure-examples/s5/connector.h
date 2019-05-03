#pragma once

#include <memory>
#include <uvp.hpp>

#include "s5define.h"

// --

struct JsonConfig;
class S5Acceptor;
class Connector {
  struct Impl;
  std::unique_ptr<Impl> _impl;

public:
  Connector(uvp::Loop *loop, const JsonConfig& jc);
  virtual ~Connector();

  uvp::Loop *loop() const;
  S5Acceptor* acceptor() const;
  void write(S5Record::Type t, const std::string &from, const uint8_t *p,
             size_t len);
};
