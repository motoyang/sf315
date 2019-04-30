#pragma once

#include <memory>
#include <uvp.hpp>

#include "s5define.h"

// --

class S5Acceptor;
class S5Peer {
  struct Impl;
  std::unique_ptr<Impl> _impl;

public:
  S5Peer(uvp::Loop *loop, S5Acceptor *acceptor);
  virtual ~S5Peer();

  uvp::Tcp *socket() const;
  std::string peer();
  void shutdown();
  void write(const uint8_t *p, size_t len);
};

// --

class Connector;
class S5Acceptor {
  struct Impl;
  std::unique_ptr<Impl> _impl;

public:
  S5Acceptor(Connector *conn, const struct sockaddr *addr);
  virtual ~S5Acceptor();

  void clientsShutdown();
  std::unique_ptr<S5Peer> removeClient(const std::string &name);
  void shutdown();
  void shutdown(const std::string &from);
  void writeToConnector(S5Record::Type t, const std::string &from, const uint8_t *p,
             size_t len);
  void writeToPeer(const std::string &from, const uint8_t *p, size_t len);
};

// --
