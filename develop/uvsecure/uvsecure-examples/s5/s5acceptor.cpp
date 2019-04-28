#include <string>
#include <list>
#include <vector>
#include <unordered_map>

#include "cryptography.h"
#include "connector.h"
// #include "s5connector.h"
#include "s5acceptor.h"

// --

class S5Peer::Impl {
  uvp::TcpObject _socket;
  std::string _peer;
  S5Acceptor *_acceptor = nullptr;

  int p = 0;

  void onHello(const char *p, size_t len);
  void onRequest(const char *p, size_t len);
  void onRead(uvp::Stream *stream, ssize_t nread, const uvp::uv::BufT *buf);

  void onWrite(uvp::Stream *stream, int status, uvp::uv::BufT bufs[],
               int nbufs);
  void onShutdown(uvp::Stream *stream, int status);
  void onClose(uvp::Handle *handle);

public:
  Impl(uvp::Loop *loop, S5Acceptor *acceptor);

  uvp::Tcp *socket() const;
  std::string peer();
  void shutdown();
  void write(const uint8_t *p, size_t len);
};

struct S5Acceptor::Impl {
  uvp::TcpObject _socket;
  S5Acceptor *_acceptor = nullptr;
  Connector *_secureConnector = nullptr;

  std::unordered_map<std::string, std::unique_ptr<S5Peer>> _clients;
  // std::unordered_map<std::string, std::unique_ptr<TcpConn>> _connectors;

  mutable std::string _name;

  void clientsShutdown();
  void onConnection(uvp::Stream *stream, int status);
  void onShutdown(uvp::Stream *stream, int status);
  void onClose(uvp::Handle *handle);
  void addClient(std::unique_ptr<S5Peer> &&client);
  std::unique_ptr<S5Peer> removeClient(const std::string &name);

public:
  Impl(uvp::Loop *loop, const struct sockaddr *addr, S5Acceptor *acceptor);

  std::string name() const;
  void shutdown();
  void shutdown(const std::string &from);
  void secureConnector(Connector *conn);
  void write(S5Record::Type t, const std::string &from, const uint8_t *p,
             size_t len);
  void reply(const std::string &from, const uint8_t *p, size_t len);
};

// --

void S5Acceptor::Impl::clientsShutdown() {
  for (auto &c : _clients) {
    int r = c.second->impl()->socket()->readStop();
    UVP_LOG_ERROR(r);
    c.second->impl()->shutdown();
  }
}

void S5Acceptor::Impl::onConnection(uvp::Stream *stream, int status) {
  if (status < 0) {
    UVP_LOG_ERROR(status);
    return;
  }

  auto client = std::make_unique<S5Peer>(_socket.loop(), _acceptor);
  int r = _socket.accept(client->impl()->socket());
  if (r < 0) {
    UVP_LOG_ERROR(r);
    return;
  }
  client->impl()->socket()->readStart();

  LOG_INFO << "accept connection from: " << client->impl()->peer();
  addClient(std::move(client));
}

void S5Acceptor::Impl::onShutdown(uvp::Stream *stream, int status) {
  if (status < 0) {
    UVP_LOG_ERROR(status);
  }
  LOG_INFO << "listen socket shutdown.";

  // 这是一个listen socket，没有onRead函数，在此处直接close handle。
  _socket.close();
}

void S5Acceptor::Impl::onClose(uvp::Handle *handle) {
  LOG_INFO << "handle of listen socket closed.";
}

void S5Acceptor::Impl::addClient(std::unique_ptr<S5Peer> &&client) {
  std::string n = client->impl()->peer();

  if (auto i = _clients.insert({n, std::forward<decltype(client)>(client)});
      !i.second) {
    LOG_CRIT << "client agent has a same name: " << n;
  }
  LOG_INFO << "add client: " << n;
}

std::unique_ptr<S5Peer>
S5Acceptor::Impl::removeClient(const std::string &name) {
  auto i = _clients.find(name);
  if (i == _clients.end()) {
    LOG_CRIT << "can't find the client: " << name;
    return std::unique_ptr<S5Peer>();
  }
  auto p = std::move(i->second);

  int num = _clients.erase(name);
  LOG_INFO << "remove " << num << " client: " << name;

  return p;
}

S5Acceptor::Impl::Impl(uvp::Loop *loop, const struct sockaddr *addr,
                       S5Acceptor *acceptor)
    : _socket(loop), _acceptor(acceptor) {
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

std::string S5Acceptor::Impl::name() const {
  if (_name.empty()) {
    _name = _socket.name();
  }
  return _name;
}

void S5Acceptor::Impl::shutdown() {
  int r = _socket.shutdown();
  if (r && !_socket.isClosing()) {
    UVP_LOG_ERROR(r);
    _socket.close();
  }
}

void S5Acceptor::Impl::shutdown(const std::string &from) {
  if (auto it = _clients.find(from); it != _clients.end()) {
    it->second->impl()->shutdown();
  }
}

void S5Acceptor::Impl::secureConnector(Connector *conn) {
  _secureConnector = conn;
}

void S5Acceptor::Impl::write(S5Record::Type t, const std::string &from,
                             const uint8_t *p, size_t len) {
  _secureConnector->secureWrite(t, from, p, len);
}

void S5Acceptor::Impl::reply(const std::string &from, const uint8_t *p,
                             size_t len) {
  if (auto it = _clients.find(from); it != _clients.end()) {
    it->second->impl()->write(p, len);
  }
}

// --

void S5Peer::Impl::onHello(const char *p, size_t len) {
  if (len <= sizeof(Hello)) {
    return;
  }
  auto h = (Hello *)p;
  if (len < h->size()) {
    return;
  }
  if (h->ver != 5) {
    shutdown();
  }

  uint8_t r[2] = {5, 0};
  write(r, sizeof(r));
}

void S5Peer::Impl::onRequest(const char *p, size_t len) {
  if (len < sizeof(Request)) {
    return;
  }
  auto req = (Request *)p;
  if (len < req->size()) {
    return;
  }

  _acceptor->impl()->write(S5Record::Type::Request, peer(), (const uint8_t *)p,
                           len);
}

void S5Peer::Impl::onRead(uvp::Stream *stream, ssize_t nread,
                          const uvp::uv::BufT *buf) {
  if (nread < 0) {
    _socket.close();
    UVP_LOG_ERROR(nread);

    uint8_t data[1] = {0};
    _acceptor->impl()->write(S5Record::Type::S5PeerClosed, peer(), data,
                             sizeof(data));

    return;
  }

  if (nread) {
    std::string s = secure::hex_encode((const uint8_t *)buf->base, nread);
    std::cout << "recv: " << s << std::endl;

    if (p == 0)
      onHello(buf->base, nread);
    if (p == 1)
      onRequest(buf->base, nread);
    if (p > 1) {
      _acceptor->impl()->write(S5Record::Type::Data, peer(),
                               (const uint8_t *)buf->base, nread);
    }
    ++p;
  }
}

void S5Peer::Impl::onWrite(uvp::Stream *stream, int status,
                           uvp::uv::BufT bufs[], int nbufs) {
  if (status < 0) {
    UVP_LOG_ERROR(status);
  }

  for (int i = 0; i < nbufs; ++i) {
    uvp::freeBuf(bufs[i]);
  }
}

void S5Peer::Impl::onShutdown(uvp::Stream *stream, int status) {
  if (status < 0) {
    UVP_LOG_ERROR(status);
    if (!_socket.isClosing()) {
      _socket.close();
    }
  }
  LOG_INFO << "s5peer socket shutdown." << peer();
}

void S5Peer::Impl::onClose(uvp::Handle *handle) {
  // 需要在onClose中先hold住clientagent，否则对象被销毁，onClose代码执行非法！
  auto p = _acceptor->impl()->removeClient(_peer);
  LOG_INFO << "handle of s5peer socket closed." << _peer;
}

S5Peer::Impl::Impl(uvp::Loop *loop, S5Acceptor *acceptor)
    : _socket(loop), _acceptor(acceptor) {
  _socket.writeCallback(std::bind(
      &S5Peer::Impl::onWrite, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
  _socket.readCallback(std::bind(&S5Peer::Impl::onRead, this,
                                 std::placeholders::_1, std::placeholders::_2,
                                 std::placeholders::_3));
  _socket.shutdownCallback(std::bind(&S5Peer::Impl::onShutdown, this,
                                     std::placeholders::_1,
                                     std::placeholders::_2));
  _socket.closeCallback(
      std::bind(&S5Peer::Impl::onClose, this, std::placeholders::_1));
}

uvp::Tcp *S5Peer::Impl::socket() const { return (uvp::Tcp *)&_socket; }

std::string S5Peer::Impl::peer() {
  if (_peer.empty()) {
    _peer = _socket.peer();
  }
  return _peer;
}

void S5Peer::Impl::shutdown() {
  int r = _socket.shutdown();
  if (r && !_socket.isClosing()) {
    UVP_LOG_ERROR(r);
    _socket.close();
  }
}

void S5Peer::Impl::write(const uint8_t *p, size_t len) {
  auto b = uvp::copyToBuf((const char *)p, len);
  int r = _socket.write(&b, 1);
  if (r) {
    uvp::freeBuf(b);
    UVP_LOG_ERROR(r);
  }
}

// --

S5Peer::S5Peer(uvp::Loop *loop, S5Acceptor *acceptor)
    : _impl(std::make_unique<S5Peer::Impl>(loop, acceptor)) {}

S5Peer::~S5Peer() {}

S5Peer::Impl *S5Peer::impl() const { return _impl.get(); }

// --

S5Acceptor::S5Acceptor(uvp::Loop *loop, const struct sockaddr *addr)
    : _impl(std::make_unique<S5Acceptor::Impl>(loop, addr, this)) {}

S5Acceptor::~S5Acceptor() {}

S5Acceptor::Impl *S5Acceptor::impl() const { return _impl.get(); }

std::string S5Acceptor::name() const { return _impl->name(); }

void S5Acceptor::secureConnector(Connector *conn) {
  _impl->secureConnector(conn);
}

void S5Acceptor::shutdown() { _impl->shutdown(); }

void S5Acceptor::shutdown(const std::string &from) { _impl->shutdown(from); }

void S5Acceptor::reply(const std::string &from, const uint8_t *p, size_t len) {
  _impl->reply(from, p, len);
}
