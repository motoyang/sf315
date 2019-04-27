#include "acceptor.h"
#include "s5connector.h"

// --

void S5Connector::onRead(uvp::Stream *stream, ssize_t nread,
                         const uvp::uv::BufT *buf) {
  if (nread < 0) {
    _socket.close();
    UVP_LOG_ERROR(nread);
    return;
  }

  if (nread) {
    _peer->write(from(), (const uint8_t *)buf->base, nread);
  }
}

void S5Connector::onResolver(uvp::Getaddrinfo *req, int status, addrinfo *res) {
  if (status < 0) {
    std::cerr << "getaddrinfo callback error: " << uvp::Error(status).strerror()
              << std::endl;
    return;
  }

  _socket.connect((const sockaddr *)res->ai_addr);
  delete req;
}

void S5Connector::onConnect(uvp::Stream *stream, int status) {
#pragma pack(1)
  struct Reply {
    uint8_t ver;
    uint8_t reply;
    uint8_t rsv;
    uint8_t atype;
    // uint8_t len;
    auto addr() const { return (char *)(this + 1); }
    auto port() const {
      uint16_t p = *(uint16_t *)(addr() + 4);
      return ntohs(p);
    }
    void port(uint16_t p) { *(uint16_t *)(addr() + 4) = htons(p); }
    auto size() const { return sizeof(*this) + 4 + sizeof(uint16_t); }
  };
#pragma pack()

  if (status < 0) {
    return;
  }

  _socket.readStart();

  sockaddr addr = {0};
  sockaddr_in *paddr_in = (sockaddr_in *)&addr;
  int len = sizeof(addr);
  int r2 = _socket.getsockname(&addr, &len);
  UVP_LOG_ERROR(r2);

  std::vector<uint8_t> buf(256, 0);
  auto r = (Reply *)buf.data();
  r->ver = 5;
  r->reply = 0;
  r->rsv = 0;
  r->atype = 1;
  // r->len = 4;
  memcpy(r->addr(), &paddr_in->sin_addr, 4);
  r->port(paddr_in->sin_port);

  buf.resize(r->size());
  _peer->write(from(), buf.data(), buf.size());

  // auto it = _acceptor->impl()->_clients.find(_peer);
  // if (it != _acceptor->impl()->_clients.end()) {
  //   it->second->write(buf.data(), buf.size());
  // }
}

// --
