#pragma once

#include <memory>
#include <uvp.hpp>

#include "s5define.h"

// --

class Acceptor;
class S5Connector;

class TcpPeer {
  struct Impl;
  std::unique_ptr<Impl> _impl;

public:
  TcpPeer(uvp::Loop *loop, Acceptor *acceptor);
  virtual ~TcpPeer();

  uvp::Tcp *socket() const;
  void sayHello();
  void write(S5Record::Type t, const std::string &from, const uint8_t *p, size_t len);
  std::unique_ptr<S5Connector> removeConnector(const std::string &name);
};

// --

struct JsonConfig;
class Acceptor {
  struct Impl;
  std::unique_ptr<Impl> _impl;

public:
  Acceptor(uvp::Loop *loop, const JsonConfig& jc);
  virtual ~Acceptor();

  std::unique_ptr<TcpPeer> removeClient(const std::string &name);
  const JsonConfig* jsonConfig() const;
};
