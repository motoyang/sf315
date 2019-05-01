#include <unordered_map>

#include <uvplus.hpp>

#include "s5define.h"
#include "cryptography.h"
#include "securelayer.h"
#include "s5connector.h"
#include "acceptor.h"

// --

class TcpPeer::Impl {
  uvp::TcpObject _socket;
  TcpPeer *_peer = nullptr;
  Acceptor *_acceptor = nullptr;

  // SecureRecord _sr;
    SsRecord _sr;
  std::unordered_map<std::string, std::unique_ptr<S5Connector>> _connectors;

  void request1(const std::string &from, const Request *req);
  void request2(const std::string &from, const uint8_t *p, size_t len);
  void doSomething(const u8vector &v);
  void collect(const char *p, size_t len);
  void onRead(uvp::Stream *stream, ssize_t nread, const uvp::uv::BufT *buf);
  void onWrite(uvp::Stream *stream, int status, uvp::uv::BufT bufs[],
               int nbufs);
  void onShutdown(uvp::Stream *stream, int status);
  void onClose(uvp::Handle *handle);
  void writeRecord(const uint8_t *p, size_t len);
  void writeChunks(const std::list<uvp::uv::BufT> &l);

public:
  Impl(uvp::Loop *loop, Acceptor *acceptor, TcpPeer *peer, bool secure);

  uvp::Tcp *socket() const { return (uvp::Tcp *)&_socket; }
  // void sayHello();
  std::unique_ptr<S5Connector> removeConnector(const std::string &name);
  void write(S5Record::Type t, const std::string &from, const uint8_t *p,
             size_t len);
};

// --

class Acceptor::Impl {
  uvp::TcpObject _socket;
  Acceptor *_acceptor;
  std::unordered_map<std::string, std::unique_ptr<TcpPeer>> _clients;
  mutable std::string _name;
  bool _secure;

  void onConnection(uvp::Stream *stream, int status);
  void onShutdown(uvp::Stream *stream, int status);
  void onClose(uvp::Handle *handle);
  std::string name() const;

public:
  Impl(uvp::Loop *loop, const struct sockaddr *addr, Acceptor *acceptor,
       bool secure);

  void addClient(std::unique_ptr<TcpPeer> &&client);
  std::unique_ptr<TcpPeer> removeClient(const std::string &name);
};

// --

void TcpPeer::Impl::request1(const std::string &from, const Request *req) {
  std::string url(req->addr(), req->len);
  std::string port(8, 0);
  std::sprintf(port.data(), "%d", req->port());

  addrinfo hints = {0};
  hints.ai_family = PF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = 0;

  auto conn = std::make_unique<S5Connector>(_socket.loop(), _peer, from);

  uvp::GetaddrinfoReq *resolver = new uvp::GetaddrinfoReq;
  int r = _socket.loop()->getAddrInfo(
      resolver,
      std::bind(&S5Connector::onResolver, conn.get(), std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3),
      url.c_str(), port.c_str(), &hints);
  if (r) {
    std::cerr << "getaddrinfo call error: " << uvp::Error(r).strerror()
              << std::endl;
  }
  _connectors.insert({from, std::move(conn)});
}

void TcpPeer::Impl::request2(const std::string &from, const uint8_t *p,
                             size_t len) {
  if (auto it = _connectors.find(from); it != _connectors.end()) {
    it->second->write(p, len);
  } else {
    // 通知local侧，s5connector已经关闭，local侧需要关闭对应的s5peer。
    uint8_t data[1] = {0};
    _peer->write(S5Record::Type::S5ConnectorClosed, from, data, sizeof(data));
  }
}

void TcpPeer::Impl::doSomething(const u8vector &v) {
  static size_t count = 0;
  std::cout << "received " << v.size() << " bytes from: " << _socket.peer()
            << ", Records: " << ++count << std::endl;
  std::cout << secure::hex_encode(v) << std::endl;

  auto s5r = (S5Record *)v.data();
  std::string from(s5r->from()->data(), s5r->from()->len());
  if (s5r->type == S5Record::Type::Request) {
    request1(from, (Request *)s5r->data()->data());
  }
  if (s5r->type == S5Record::Type::Data) {
    request2(from, s5r->data()->data(), s5r->data()->len());
  }
}

void TcpPeer::Impl::collect(const char *p, size_t len) {
  auto lv = _sr.feed(p, len);
  for (auto v : lv) {
    doSomething(v);
  }
}

void TcpPeer::Impl::onRead(uvp::Stream *stream, ssize_t nread,
                           const uvp::uv::BufT *buf) {
  if (nread < 0) {
    _socket.close();
    UVP_LOG_ERROR(nread);
    return;
  }

  // 整理包，并进一步处理包
  if (nread) {
    // std::cout << "received " << nread << " bytes from: " << _socket.peer()
    //           << std::endl;
    // std::cout << secure::hex_encode(u8vector(buf->base, buf->base + nread)) << std::endl;
    collect(buf->base, nread);
  }
}

void TcpPeer::Impl::onWrite(uvp::Stream *stream, int status,
                            uvp::uv::BufT bufs[], int nbufs) {
  if (status < 0) {
    UVP_LOG_ERROR(status);
  }

  for (int i = 0; i < nbufs; ++i) {
    uvp::freeBuf(bufs[i]);
  }
}

void TcpPeer::Impl::onShutdown(uvp::Stream *stream, int status) {
  if (status < 0) {
    UVP_LOG_ERROR(status);
    if (!_socket.isClosing()) {
      _socket.close();
    }
  }
  LOG_INFO << "peer socket shutdown." << _socket.peer();
}

void TcpPeer::Impl::onClose(uvp::Handle *handle) {
  // 需要在onClose中先hold住clientagent，否则对象被销毁，onClose代码执行非法！
  std::string peer = _socket.peer();
  auto p = _acceptor->removeClient(peer);
  LOG_INFO << "handle of peer socket closed. " << peer;
}

void TcpPeer::Impl::writeRecord(const uint8_t *p, size_t len) {
  if (_sr.isExpired()) {
    auto l = _sr.update();
    writeChunks(l);
  }
  auto l = _sr.pack(p, len);
  writeChunks(l);
}

void TcpPeer::Impl::writeChunks(const std::list<uvp::uv::BufT> &l) {
  for (auto b : l) {
    int r = _socket.write(&b, 1);
    if (r) {
      uvp::freeBuf(b);
      UVP_LOG_ERROR(r);
    }
  }
}

TcpPeer::Impl::Impl(uvp::Loop *loop, Acceptor *acceptor, TcpPeer *peer,
                    bool secure)
    : _sr(secure), _socket(loop), _acceptor(acceptor), _peer(peer) {
  _socket.writeCallback(std::bind(
      &TcpPeer::Impl::onWrite, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
  _socket.readCallback(std::bind(&TcpPeer::Impl::onRead, this,
                                 std::placeholders::_1, std::placeholders::_2,
                                 std::placeholders::_3));
  _socket.shutdownCallback(std::bind(&TcpPeer::Impl::onShutdown, this,
                                     std::placeholders::_1,
                                     std::placeholders::_2));
  _socket.closeCallback(
      std::bind(&TcpPeer::Impl::onClose, this, std::placeholders::_1));
}
/*
void TcpPeer::Impl::sayHello() {
  auto l = _sr.reset();
  writeChunks(l);
}
*/
std::unique_ptr<S5Connector>
TcpPeer::Impl::removeConnector(const std::string &name) {
  // 将s5connector从列表中移除
  auto i = _connectors.find(name);
  if (i == _connectors.end()) {
    LOG_CRIT << "can't find the client: " << name;
    return std::unique_ptr<S5Connector>();
  }
  auto p = std::move(i->second);

  int num = _connectors.erase(name);
  LOG_INFO << "remove " << num << " client: " << name;

  return p;
}

void TcpPeer::Impl::write(S5Record::Type t, const std::string &from,
                          const uint8_t *p, size_t len) {
  UVP_ASSERT(len > 0);
  while (len > 0) {
    auto writeLen =
        std::min(_sr.length() - S5Record::HeadLen() - from.size(), len);
    u8vector v(S5Record::HeadLen() + from.size() + writeLen);
    auto s5r = (S5Record *)v.data();

    s5r->type = t;
    s5r->from()->len(from.size());
    std::memcpy(s5r->from()->data(), from.data(), from.size());
    s5r->data()->len(writeLen);
    std::memcpy(s5r->data()->data(), p, writeLen);

    len -= writeLen;
    p += writeLen;

    UVP_ASSERT(s5r->size() == v.size());
    writeRecord(v.data(), v.size());
  }
}

// --

Acceptor::Impl::Impl(uvp::Loop *loop, const struct sockaddr *addr,
                     Acceptor *acceptor, bool secure)
    : _secure(secure), _socket(loop), _acceptor(acceptor) {
  const int backlog = 128;

  _socket.connectionCallback(std::bind(
      &Impl::onConnection, this, std::placeholders::_1, std::placeholders::_2));
  _socket.shutdownCallback(std::bind(
      &Impl::onShutdown, this, std::placeholders::_1, std::placeholders::_2));
  _socket.closeCallback(std::bind(&Impl::onClose, this, std::placeholders::_1));

  int r = _socket.bind(addr, 0);
  UVP_LOG_ERROR_EXIT(r);
  r = _socket.listen(backlog);
  UVP_LOG_ERROR_EXIT(r);

  LOG_INFO << "listen at: " << name();
}

void Acceptor::Impl::onConnection(uvp::Stream *stream, int status) {
  if (status < 0) {
    UVP_LOG_ERROR(status);
    return;
  }

  auto client = std::make_unique<TcpPeer>(_socket.loop(), _acceptor, _secure);
  int r = _socket.accept(client->socket());
  if (r < 0) {
    UVP_LOG_ERROR(r);
    return;
  }
  client->socket()->readStart();
  // client->sayHello();

  LOG_INFO << "accept connection from: " << client->socket()->peer();
  addClient(std::move(client));
}

void Acceptor::Impl::onShutdown(uvp::Stream *stream, int status) {
  if (status < 0) {
    UVP_LOG_ERROR(status);
  }
  LOG_INFO << "listen socket shutdown.";
  _socket.close();
}

void Acceptor::Impl::onClose(uvp::Handle *handle) {
  LOG_INFO << "handle of listen socket closed.";
}

std::string Acceptor::Impl::name() const {
  if (_name.empty()) {
    _name = _socket.name();
  }
  return _name;
}

void Acceptor::Impl::addClient(std::unique_ptr<TcpPeer> &&client) {
  std::string n = client->socket()->peer();

  if (auto i = _clients.insert({n, std::forward<decltype(client)>(client)});
      !i.second) {
    LOG_CRIT << "client agent has a same name: " << n;
  }
  LOG_INFO << "add client: " << n;
}

std::unique_ptr<TcpPeer> Acceptor::Impl::removeClient(const std::string &name) {
  auto i = _clients.find(name);
  if (i == _clients.end()) {
    LOG_CRIT << "can't find the client: " << name;
    return std::unique_ptr<TcpPeer>();
  }
  auto p = std::move(i->second);

  int num = _clients.erase(name);
  LOG_INFO << "remove " << num << " client: " << name;

  return p;
}

// --

TcpPeer::TcpPeer(uvp::Loop *loop, Acceptor *acceptor, bool secure)
    : _impl(std::make_unique<TcpPeer::Impl>(loop, acceptor, this, secure)) {}

TcpPeer::~TcpPeer() {}

uvp::Tcp *TcpPeer::socket() const { return _impl->socket(); }

// void TcpPeer::sayHello() { return _impl->sayHello(); }

void TcpPeer::write(S5Record::Type t, const std::string &from, const uint8_t *p,
                    size_t len) {
  _impl->write(t, from, p, len);
}

std::unique_ptr<S5Connector> TcpPeer::removeConnector(const std::string &name) {
  return _impl->removeConnector(name);
}

// --

Acceptor::Acceptor(uvp::Loop *loop, const struct sockaddr *addr, bool secure)
    : _impl(std::make_unique<Acceptor::Impl>(loop, addr, this, secure)) {}

Acceptor::~Acceptor() {}

std::unique_ptr<TcpPeer> Acceptor::removeClient(const std::string &name) {
  return _impl->removeClient(name);
}
