#pragma once

#include <memory>

// --

// struct TransportInterface;

class Channel {
  // std::shared_ptr<TransportInterface> _ti;

  struct Impl;
  std::unique_ptr<Impl> _impl;

public:
  Channel();
  virtual ~Channel();

  bool send(uint8_t *p, size_t len) const;
  secure::secure_vector<uint8_t> recv() const;
  void bind(Channel *peer);
//   bool sendHandshake(const Handshake* hs);
//   bool sendAlert();
//   bool sendCipher();
//   bool sendAppdata();
};
