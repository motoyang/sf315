#pragma once

#include <memory>
#include <stack>
#include <unordered_map>

// --

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

  struct Impl;
  std::unique_ptr<Impl> _impl;

  Handshake *hello(std::vector<uint8_t> &buf, const ClientHello *hello);

  void sayHello(const Handshake *hs);
  void sayEncryptedExtensions();
  void sayAlert(AlertDescription desc, AlertLevel level = AlertLevel::fatal);
  void sendFragment(ContentType ct, const uint8_t* p, size_t len) const;

  bool recvFragment(ContentType& ct, secure::secure_vector<uint8_t>& buf) const;
  void received(ContentType ct, const uint8_t *p, size_t len);
  // void received(const Handshake *hs, size_t len);
  // void received(const Alert *alert, size_t len);
  // void received(const uint8_t *appdata, size_t len);

  std::unordered_map<ExtensionType, uint8_t *> extensionsCheck(Extensions *e);

public:
  Server();
  virtual ~Server();

  // 测试用的方法，libuv加入后，删除此方法。
  Channel* channel() const;

  void run();
};