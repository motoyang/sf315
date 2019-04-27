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

  SecureRecord _sr;
  size_t _recordCount = 0;

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

public:
  Impl(uvp::Loop *loop, Acceptor *acceptor, TcpPeer *peer, bool secure);

  uvp::Tcp *socket() const { return (uvp::Tcp *)&_socket; }
  void write(const uint8_t *p, size_t len);
  void sayHello();
  void write(const std::list<uvp::uv::BufT> &l);
  size_t length() const;
};

// --

struct Acceptor::Impl {
private:
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
    UVP_ASSERT(false);
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

  // write(_sr.pack(v.data(), v.size()));
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
  }
  LOG_INFO << "agent socket shutdown." << _socket.peer();
  _socket.close();
}

void TcpPeer::Impl::onClose(uvp::Handle *handle) {
  // 需要在onClose中先hold住clientagent，否则对象被销毁，onClose代码执行非法！
  auto p = _acceptor->impl()->removeClient(_socket.peer());
  LOG_INFO << "handle of agent socket closed." << _socket.peer();
}

void TcpPeer::Impl::write(const uint8_t *p, size_t len) {
  if (++_recordCount % 73 == 0) {
    auto l = _sr.update();
    write(l);
  }
  auto l = _sr.pack(p, len);
  write(l);
}

void TcpPeer::Impl::sayHello() {
  auto l = _sr.reset();
  write(l);
}

void TcpPeer::Impl::write(const std::list<uvp::uv::BufT> &l) {
  for (auto b : l) {
    int r = _socket.write(&b, 1);
    if (r) {
      uvp::freeBuf(b);
      UVP_LOG_ERROR(r);
    }
  }
}

size_t TcpPeer::Impl::length() const { return _sr.length() - 256 - 2 - 64; }

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
  int r = _socket.accept(client->impl()->socket());
  if (r < 0) {
    UVP_LOG_ERROR(r);
    return;
  }
  client->impl()->socket()->readStart();
  client->impl()->sayHello();

  LOG_INFO << "accept connection from: " << client->impl()->socket()->peer();
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
  std::string n = client->impl()->socket()->peer();

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

TcpPeer::Impl *TcpPeer::impl() const { return _impl.get(); }

void TcpPeer::write(const std::string &from, const uint8_t *p, size_t len) {

  while (len > 0) {
    auto writeLen = std::min(_impl->length(), len);
    UVP_ASSERT(writeLen < 18 * 1024);

    // auto from = _impl->from();
    u8vector v(S5Record::HeadLen() + from.size() + writeLen);
    auto s5r = (S5Record *)v.data();

    s5r->type = S5Record::Type::Reply;
    s5r->from()->len(from.size());
    std::memcpy(s5r->from()->data(), from.data(), from.size());
    s5r->data()->len(writeLen);
    std::memcpy(s5r->data()->data(), p, writeLen);
    auto l1 = s5r->size();
    auto l2 = v.size();

    UVP_ASSERT(s5r->size() == v.size());

    // v.resize(s5r->size());
    _impl->write(v.data(), v.size());

    len -= writeLen;
    p += writeLen;
  }
}

// --

Acceptor::Acceptor(uvp::Loop *loop, const struct sockaddr *addr, bool secure)
    : _impl(std::make_unique<Acceptor::Impl>(loop, addr, this, secure)) {}

Acceptor::~Acceptor() {}

Acceptor::Impl *Acceptor::impl() const { return _impl.get(); }