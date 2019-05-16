#include <string>
#include <list>
#include <vector>
#include <unordered_map>

#include "uvplus.hpp"
#include "cryptography.h"
#include "connector.h"
#include "s5acceptor.h"

// --

class S5Peer::Impl {
  uvp::TcpObject _socket;
  std::string _peer;
  S5Acceptor *_acceptor = nullptr;

  int _count = 0;
  u8vector _buffer;

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

class S5Acceptor::Impl {
  uvp::TcpObject _socket;
  Connector *_connector = nullptr;

  std::unordered_map<std::string, std::unique_ptr<S5Peer>> _clients;
  mutable std::string _name;

  void onConnection(uvp::Stream *stream, int status);
  void onShutdown(uvp::Stream *stream, int status);
  void onClose(uvp::Handle *handle);
  void addClient(std::unique_ptr<S5Peer> &&client);
  std::string name() const;

public:
  Impl(Connector *conn, const struct sockaddr *addr);

  void clientsShutdown();
  std::unique_ptr<S5Peer> removeClient(const std::string &name);
  void shutdown();
  void shutdown(const std::string &from);
  void writeToConnector(S5Record::Type t, const std::string &from,
                        const uint8_t *p, size_t len);
  void writeToPeer(const std::string &from, const uint8_t *p, size_t len);
};

// --

void S5Acceptor::Impl::clientsShutdown() {
  for (auto &c : _clients) {
    int r = c.second->socket()->readStop();
    UVP_LOG_ERROR(r);
    c.second->shutdown();
  }
}

void S5Acceptor::Impl::onConnection(uvp::Stream *stream, int status) {
  if (status < 0) {
    UVP_LOG_ERROR(status);
    return;
  }

  auto client =
      std::make_unique<S5Peer>(_socket.loop(), _connector->acceptor());
  int r = _socket.accept(client->socket());
  if (r < 0) {
    UVP_LOG_ERROR(r);
    return;
  }
  client->socket()->readStart();

  LOG_INFO << "accept connection from: " << client->peer();
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
  std::string n = client->peer();

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

S5Acceptor::Impl::Impl(Connector *conn, const struct sockaddr *addr)
    : _socket(conn->loop()), _connector(conn) {
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
    it->second->shutdown();
  }
}

void S5Acceptor::Impl::writeToConnector(S5Record::Type t,
                                        const std::string &from,
                                        const uint8_t *p, size_t len) {
  _connector->write(t, from, p, len);
}

void S5Acceptor::Impl::writeToPeer(const std::string &from, const uint8_t *p,
                                   size_t len) {
  if (auto it = _clients.find(from); it != _clients.end()) {
    it->second->write(p, len);
  }
}

// --

void S5Peer::Impl::onHello(const char *p, size_t len) {
  _buffer.insert(_buffer.end(), p, p + len);
  if (_buffer.size() <= sizeof(Hello)) {
    return;
  }
  auto h = (Hello *)_buffer.data();
  if (_buffer.size() < h->size()) {
    return;
  }
  if (h->ver != 5) {
    shutdown();
  }

  uint8_t r[2] = {5, 0};
  write(r, sizeof(r));

  if (_buffer.size() > h->size()) {
    // 出现了粘包的情况，需要将未处理的数据保存下来，下次继续处理
    u8vector tmp(_buffer.data() + h->size(), _buffer.data() + _buffer.size());
    _buffer = tmp;
  } else {
    _buffer.clear();
  }
}

void S5Peer::Impl::onRequest(const char *p, size_t len) {
  _buffer.insert(_buffer.end(), p, p + len);
  if (_buffer.size() < sizeof(Request)) {
    return;
  }
  auto req = (Request *)_buffer.data();
  if (_buffer.size() < req->size()) {
    return;
  }

  _acceptor->writeToConnector(S5Record::Type::Request, peer(),
                              (const uint8_t *)req, req->size());

  if (_buffer.size() > req->size()) {
    // 出现了粘包的情况，需要将未处理的数据保存下来，下次继续处理
    u8vector tmp(_buffer.data() + req->size(), _buffer.data() + _buffer.size());
    _buffer = tmp;
  } else {
    _buffer.clear();
  }
}

void S5Peer::Impl::onRead(uvp::Stream *stream, ssize_t nread,
                          const uvp::uv::BufT *buf) {
  if (nread < 0) {
    _socket.close();
    UVP_LOG_ERROR(nread);

    // 通知remote侧，s5peer已经关闭，remote侧需要关闭对应的s5connector。
    uint8_t data[1] = {0};
    _acceptor->writeToConnector(S5Record::Type::S5PeerClosed, peer(), data,
                                sizeof(data));

    return;
  }

  if (nread) {
    std::string s = secure::hex_encode((const uint8_t *)buf->base, nread);
    std::cout << "recv: " << s << std::endl;

    if (_count == 0) {
      onHello(buf->base, nread);
    } else if (_count == 1) {
      onRequest(buf->base, nread);
    } else {
      if (!_buffer.empty()) {
        // 有粘包，上次未处理的数据要在本次一起处理
        _buffer.insert(_buffer.end(), buf->base, buf->base + nread);
        _acceptor->writeToConnector(S5Record::Type::Data, peer(),
                                    (const uint8_t *)_buffer.data(),
                                    _buffer.size());
        _buffer.clear();
      } else {
        _acceptor->writeToConnector(S5Record::Type::Data, peer(),
                                    (const uint8_t *)buf->base, nread);
      }
    }
    ++_count;
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
  auto p = _acceptor->removeClient(_peer);
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

// S5Peer::Impl *S5Peer::impl() const { return _impl.get(); }

uvp::Tcp *S5Peer::socket() const { return _impl->socket(); }

std::string S5Peer::peer() { return _impl->peer(); }

void S5Peer::shutdown() { return _impl->shutdown(); }

void S5Peer::write(const uint8_t *p, size_t len) {
  return _impl->write(p, len);
}

// --

S5Acceptor::S5Acceptor(Connector *conn, const struct sockaddr *addr)
    : _impl(std::make_unique<S5Acceptor::Impl>(conn, addr)) {}

S5Acceptor::~S5Acceptor() {}

void S5Acceptor::clientsShutdown() { _impl->clientsShutdown(); }

void S5Acceptor::shutdown() { _impl->shutdown(); }

void S5Acceptor::shutdown(const std::string &from) { _impl->shutdown(from); }

std::unique_ptr<S5Peer> S5Acceptor::removeClient(const std::string &name) {
  return _impl->removeClient(name);
}

void S5Acceptor::writeToPeer(const std::string &from, const uint8_t *p,
                             size_t len) {
  _impl->writeToPeer(from, p, len);
}

void S5Acceptor::writeToConnector(S5Record::Type t, const std::string &from,
                                  const uint8_t *p, size_t len) {
  _impl->writeToConnector(t, from, p, len);
}