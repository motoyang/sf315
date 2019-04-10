#pragma once

#include <stack>
// --

class VersionSupported;
class Cryptocenter;
class Channel;
class RecordLayer;

class Client {
  enum class Status : uint8_t {
    START = 1,
    WAIT_SH,
    WAIT_EE,
    WAIT_CERT_CR,
    WAIT_CERT,
    WAIT_CV,
    WAIT_FINISHED,
    CONNECTED
  };

  std::vector<uint8_t> _clientHello;
  std::stack<Status> _status;
  std::unique_ptr<VersionSupported> _vs;
  std::unique_ptr<RecordLayer> _rl;
  std::unique_ptr<Cryptocenter> _cryptocenter;
  std::unique_ptr<Channel> _channel;

  std::vector<uint8_t> _key;

  std::unordered_map<ExtensionType, uint8_t *> extensionsCheck(Extensions *e);

public:
  Client();
  void start();
  void run();
  void received(ContentType ct, const uint8_t *p, size_t len);
  void received(const ServerHello *sh);

  const Handshake *hello(std::vector<uint8_t> &buf) const;
  const Handshake *hello(std::vector<uint8_t> &buf,
                         const ServerHello *reques) const;

  void sayHello(const Handshake *hs);
  void sayAppdata(const uint8_t *p, size_t len);
  // void hear(const EncryptedExtensions& ee);

  void say(ContentType ct, const std::vector<uint8_t> &v);
};