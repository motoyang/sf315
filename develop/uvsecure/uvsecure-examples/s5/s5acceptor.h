#pragma once

#include <memory>
#include <uvp.hpp>

// --

class S5Acceptor;
class S5Peer {
  struct Impl;
  std::unique_ptr<Impl> _impl;

public:
  S5Peer(uvp::Loop *loop, S5Acceptor *acceptor);
  virtual ~S5Peer();

  Impl* impl() const;
};

// --

class Connector;
class S5Acceptor {
  struct Impl;
  std::unique_ptr<Impl> _impl;

public:

  S5Acceptor(uvp::Loop *loop, const struct sockaddr *addr);
  virtual ~S5Acceptor();

  Impl* impl() const;
  std::string name() const;
  void secureConnector(Connector *conn);
  void shutdown();
  void shutdown(const std::string& from);
  void reply(const std::string& from, const uint8_t* p, size_t len);
};

// --
