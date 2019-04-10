#pragma once

#include <stack>
#include <memory>

// --

class VersionSupported;
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
  std::unique_ptr<VersionSupported> _vs;
  std::unique_ptr<RecordLayer> _rl;
  std::unique_ptr<Cryptocenter> _cryptocenter;
  std::unique_ptr<Channel> _channel;

  std::vector<uint8_t> _key;

  std::unordered_map<ExtensionType, uint8_t *> extensionsCheck(Extensions *e);

public:
  Server();
  virtual ~Server() = default;

  void sayHello(const Handshake *hs);

  void run();
  void received(ContentType ct, const uint8_t *p, size_t len);
  void received(const Handshake *hs, size_t len);
  void received(const Alert *alert, size_t len);
  void received(const uint8_t *appdata, size_t len);

  void received(const ClientHello *hello);

  Handshake *hello(std::vector<uint8_t> &buf, const ClientHello *hello);
  void helloRetryRequest(ServerHello *sh) const;
};