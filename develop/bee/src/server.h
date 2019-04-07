#pragma once

#include <stack>
#include <memory>

// --

class Cryptocenter;
class Channel;
class RecordLayer;

class Server {
  enum class Status : uint8_t {
    START = 1,
    RECVD_CH,
    NEGOTIATED,
    WAIT_EOED,
    WAIT_FLIGHT2,
    WAIT_CERT,
    WAIT_CV,
    WAIT_FINISHED,
    CONNECTED
  };

  std::stack<Status> _status;
  std::unique_ptr<RecordLayer> _rl;
  std::unique_ptr<Cryptocenter> _cryptocenter;
  std::unique_ptr<Channel> _channel;

  std::vector<uint8_t> _key;

public:
  Server();
  virtual ~Server() = default;

  void run() const;
  void received(const ClientHello *hello) const;
};