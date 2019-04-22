#pragma once

#include <memory>
#include <string>
#include <list>
#include <vector>
#include <unordered_map>

#include <uvplus.hpp>

// --

class SecureAcceptor {
  struct Impl;
  std::unique_ptr<Impl> _impl;

public:
  enum class NotifyTag :uint8_t{ NT_NOTHING = 0, NT_CLOSE, NT_CLIENTS_SHUTDOWN };

  SecureAcceptor(uvp::Loop *loop, const struct sockaddr *addr);
  virtual ~SecureAcceptor();

  std::string name() const;
  bool read(uvplus::Packet &packet);
  size_t read(std::vector<uvplus::Packet> &packets);
  int write(const char *name, const uint8_t *p, size_t len);
  int notify(int tag);
};

// --
