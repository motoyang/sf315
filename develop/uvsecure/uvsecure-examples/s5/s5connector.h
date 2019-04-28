#pragma once

#include <uvp.hpp>

// --

class TcpPeer;

class S5Connector {
  struct Impl;
  std::unique_ptr<Impl> _impl;

public:
  S5Connector(uvp::Loop *loop, TcpPeer *peer, const std::string& from);
  virtual ~S5Connector();

  std::string from() const;
  void write(const uint8_t *p, size_t len);
  void shutdown();
  void onResolver(uvp::Getaddrinfo *req, int status, addrinfo *res);
};
