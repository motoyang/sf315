#pragma once

#include <memory>

// --

struct TransportInterface;

class Channel {
  std::shared_ptr<TransportInterface> _ti;

public:
  Channel();
  virtual ~Channel() = default;

  bool send(uint8_t *p, size_t len) const;
  std::vector<uint8_t> recv() const;
//   bool sendHandshake(const Handshake* hs);
//   bool sendAlert();
//   bool sendCipher();
//   bool sendAppdata();
};
