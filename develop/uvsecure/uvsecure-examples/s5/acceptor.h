#pragma once

#include <memory>
#include <uvp.hpp>

// --

class Acceptor;

class TcpPeer {
  struct Impl;
  std::unique_ptr<Impl> _impl;

public:
  TcpPeer(uvp::Loop *loop, Acceptor *acceptor, bool secure);
  virtual ~TcpPeer();

  Impl *impl() const;
  void write(const std::string &from, const uint8_t *p, size_t len);
};

// --

class Acceptor {
  struct Impl;
  std::unique_ptr<Impl> _impl;

public:
  Acceptor(uvp::Loop *loop, const struct sockaddr *addr, bool secure = true);
  virtual ~Acceptor();

  Impl *impl() const;
};
