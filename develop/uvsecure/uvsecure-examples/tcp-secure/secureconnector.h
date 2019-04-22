#pragma once

#include <memory>

// --

class SecureConnector {
  struct Impl;
  std::unique_ptr<Impl> _impl;

public:
  SecureConnector(uvp::Loop *loop, const struct sockaddr *dest);
  virtual ~SecureConnector();

  std::string name();
  std::string peer();
  void notify(int tag);
  void tcpNotifyInterface(uvplus::TcpNotifyInterface* tni);
  size_t read(u8vlist& l);
  template<typename It> size_t read(It first, size_t max);
  int write(const uint8_t *p, size_t len);
};

// --
