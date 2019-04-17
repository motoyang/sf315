#pragma once

#include <memory>
#include <stack>
#include <unordered_map>

// --

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

  struct Impl;
  std::unique_ptr<Impl> _impl;

  void received(ContentType ct, const uint8_t *p, size_t len);
  void received(const ServerHello *sh);
  void received(const EncryptedExtensions* ee);

  const Handshake *hello(std::vector<uint8_t> &buf) const;
  const Handshake *hello(std::vector<uint8_t> &buf,
                         const ServerHello *reques) const;

  void sayHello(const Handshake *hs);
  void sayAlert(AlertDescription desc, AlertLevel level = AlertLevel::fatal);
  void sayAppdata(const uint8_t *p, size_t len);
  void sendFragment(ContentType ct, const uint8_t *p, size_t len) const;
  bool recvFragment(ContentType& ct, secure::secure_vector<uint8_t>& buf) const;

  std::unordered_map<ExtensionType, uint8_t *> extensionsCheck(Extensions *e);

public:
  Client();
  virtual ~Client();

  // 测试用的方法，libuv加入后，删除此方法。
  Channel *channel() const;

  void start();
  void run();
};