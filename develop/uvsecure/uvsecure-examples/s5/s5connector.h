#pragma once

#include <uvp.hpp>

// --

class TcpPeer;

class S5Connector {
  uvp::TcpObject _socket;
  std::string _from;
  TcpPeer *_peer = nullptr;

  void onWrite(uvp::Stream *stream, int status, uvp::uv::BufT bufs[],
               int nbufs) {
    if (status < 0) {
      UVP_LOG_ERROR(status);
    }

    for (int i = 0; i < nbufs; ++i) {
      uvp::freeBuf(bufs[i]);
    }
  }

  void onRead(uvp::Stream *stream, ssize_t nread, const uvp::uv::BufT *buf);
  void onConnect(uvp::Stream *stream, int status);

public:
  S5Connector(uvp::Loop *loop, TcpPeer *peer, const std::string& from)
      : _socket(loop), _peer(peer), _from(from) {
    _socket.writeCallback(std::bind(
        &S5Connector::onWrite, this, std::placeholders::_1, std::placeholders::_2,
        std::placeholders::_3, std::placeholders::_4));
    _socket.readCallback(std::bind(&S5Connector::onRead, this,
                                   std::placeholders::_1, std::placeholders::_2,
                                   std::placeholders::_3));
    // _socket.shutdownCallback(std::bind(&S5Connector::onShutdown, this,
    //                                    std::placeholders::_1,
    //                                    std::placeholders::_2));
    // _socket.closeCallback(
    //     std::bind(&S5Connector::onClose, this, std::placeholders::_1));
    _socket.connectCallback(std::bind(&S5Connector::onConnect, this,
                                      std::placeholders::_1,
                                      std::placeholders::_2));
  }

  // void peer(const std::string &p) { _peer = p; }
  std::string from() const { return _from; }

  void onResolver(uvp::Getaddrinfo *req, int status, addrinfo *res);

  void write(const uint8_t *p, size_t len) {
    auto b = uvp::copyToBuf((const char *)p, len);
    int r = _socket.write(&b, 1);
    if (r) {
      uvp::freeBuf(b);
      UVP_LOG_ERROR(r);
    }
  }
};
