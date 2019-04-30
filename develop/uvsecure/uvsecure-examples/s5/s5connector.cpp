#include "s5define.h"
#include "acceptor.h"
#include "s5connector.h"

// --

struct S5Connector::Impl {
  uvp::TcpObject _socket;
  std::string _from;
  TcpPeer *_peer = nullptr;

  void onWrite(uvp::Stream *stream, int status, uvp::uv::BufT bufs[],
               int nbufs);
  void onRead(uvp::Stream *stream, ssize_t nread, const uvp::uv::BufT *buf);
  void onConnect(uvp::Stream *stream, int status);
  void onShutdown(uvp::Stream *stream, int status);
  void onClose(uvp::Handle *handle);

public:
  Impl(uvp::Loop *loop, TcpPeer *peer, const std::string &from);

  std::string from() const { return _from; }
  void shutdown();
  void write(const uint8_t *p, size_t len);
  void onResolver(uvp::Getaddrinfo *req, int status, addrinfo *res);
};

// --

void S5Connector::Impl::onWrite(uvp::Stream *stream, int status,
                                uvp::uv::BufT bufs[], int nbufs) {
  if (status < 0) {
    UVP_LOG_ERROR(status);
  }

  for (int i = 0; i < nbufs; ++i) {
    uvp::freeBuf(bufs[i]);
  }
}

void S5Connector::Impl::onRead(uvp::Stream *stream, ssize_t nread,
                               const uvp::uv::BufT *buf) {
  if (nread < 0) {
    _socket.close();
    UVP_LOG_ERROR(nread);
    return;
  }

  if (nread) {
    _peer->write(S5Record::Type::Reply, from(), (const uint8_t *)buf->base,
                 nread);
  }
}

void S5Connector::Impl::onConnect(uvp::Stream *stream, int status) {
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
  memcpy(r->addr(), &paddr_in->sin_addr, 4);
  r->port(paddr_in->sin_port);

  if (status < 0) {
    UVP_LOG_ERROR(status);
    _socket.close();

    r->reply = 1;
  } else {
    _socket.readStart();
  }

  buf.resize(r->size());
  _peer->write(S5Record::Type::Reply, from(), buf.data(), buf.size());
}

void S5Connector::Impl::onShutdown(uvp::Stream *stream, int status) {
  if (status < 0) {
    UVP_LOG_ERROR(status);
    if (!_socket.isClosing()) {
      _socket.close();
    }
  }
  LOG_INFO << "s5connector socket shutdown." << _socket.peer();
}

void S5Connector::Impl::onClose(uvp::Handle *handle) {
  // 需要在onClose中先hold住connector，否则对象被销毁，onClose代码执行非法！
  auto p = _peer->removeConnector(from());
  LOG_INFO << "handle of s5 connector closed." << from();
}

S5Connector::Impl::Impl(uvp::Loop *loop, TcpPeer *peer, const std::string &from)
    : _socket(loop), _peer(peer), _from(from) {
  _socket.writeCallback(std::bind(
      &S5Connector::Impl::onWrite, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
  _socket.readCallback(std::bind(&S5Connector::Impl::onRead, this,
                                 std::placeholders::_1, std::placeholders::_2,
                                 std::placeholders::_3));
  _socket.shutdownCallback(std::bind(&S5Connector::Impl::onShutdown, this,
                                     std::placeholders::_1,
                                     std::placeholders::_2));
  _socket.closeCallback(
      std::bind(&S5Connector::Impl::onClose, this, std::placeholders::_1));
  _socket.connectCallback(std::bind(&S5Connector::Impl::onConnect, this,
                                    std::placeholders::_1,
                                    std::placeholders::_2));
}

void S5Connector::Impl::shutdown() {
  int r = _socket.shutdown();
  if (r && !_socket.isClosing()) {
    UVP_LOG_ERROR(r);
    _socket.close();
  }
}

void S5Connector::Impl::write(const uint8_t *p, size_t len) {
  auto b = uvp::copyToBuf((const char *)p, len);
  int r = _socket.write(&b, 1);
  if (r) {
    uvp::freeBuf(b);
    UVP_LOG_ERROR(r);
  }
}

void S5Connector::Impl::onResolver(uvp::Getaddrinfo *req, int status,
                                   addrinfo *res) {
  delete req;
  req = nullptr;

  if (status < 0) {
    std::cerr << "getaddrinfo callback error: " << uvp::Error(status).strerror()
              << std::endl;
    _socket.close();
    return;
  }

  _socket.connect((const sockaddr *)res->ai_addr);
}

// --

S5Connector::S5Connector(uvp::Loop *loop, TcpPeer *peer,
                         const std::string &from)
    : _impl(std::make_unique<S5Connector::Impl>(loop, peer, from)) {}

S5Connector::~S5Connector() {}

std::string S5Connector::from() const { return _impl->from(); }

void S5Connector::write(const uint8_t *p, size_t len) { _impl->write(p, len); }

void S5Connector::shutdown() { _impl->shutdown(); }

void S5Connector::onResolver(uvp::Getaddrinfo *req, int status, addrinfo *res) {
  _impl->onResolver(req, status, res);
}