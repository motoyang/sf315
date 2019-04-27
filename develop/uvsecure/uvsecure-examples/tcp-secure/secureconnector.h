#pragma once

#include <memory>

// --

class SecureConnector {
  struct Impl;
  std::unique_ptr<Impl> _impl;

public:
  enum class NotifyTag :uint8_t{ NOTHING = 0, CLOSE, KILL };

  SecureConnector(uvp::Loop *loop, const struct sockaddr *dest, bool secure = true);
  virtual ~SecureConnector();

  std::string name();
  std::string peer();
  void notify(NotifyTag tag);
  void tcpStatusInterface(uvplus::TcpStatusInterface* tni);
  size_t read(u8vlist& l);
  int write(const uint8_t *p, size_t len);
};

// --
