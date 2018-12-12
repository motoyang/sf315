#include <iostream>
#include <atomic>

#include <utilites.hpp>
#include <misc.hpp>
#include <uv.hpp>
#include <req.hpp>

#include <singleton.hpp>
#include <threadpool.hpp>

#include "business.h"
#include "server.h"

// --

// --

void ClientAgent::makeup(const char *p, size_t len) {
  int remain = len;
  do {
    int writed = _ringbuffer.write(p, remain);
    remain -= writed;
    p += writed;

    while (true) {
      // 解析出每个包
      BufT b = _codec.decode(&_ringbuffer);
      if (!b.base) {
        break;
      }

      // 打包到packet，并上传给业务处理。
      Packet packet(_peer, b);
      _acceptor.upwardEnqueue(std::move(packet));
    }
  } while (remain > 0);
}

void ClientAgent::onRead(ssize_t nread, const BufT *buf) {
  if (nread < 0) {
    _socket.close();
    LOG_IF_ERROR(nread);
    return;
  }

  // 整理包，并进一步处理包
  if (nread) {
    makeup(buf->base, nread);
  }
}

void ClientAgent::onWrite(int status, BufT bufs[], int nbufs) {
  if (status < 0) {
    LOG_IF_ERROR(status);
  }

  for (int i = 0; i < nbufs; ++i) {
    freeBuf(bufs[i]);
  }
}

void ClientAgent::onShutdown(int status) {
  if (status < 0) {
    LOG_IF_ERROR(status);
  }
  LOG_INFO << "agent socket shutdown." << _peer;
  _socket.close();
}

void ClientAgent::onClose() {
  // 需要在onClose中先hold住clientagent，否则对象被销毁，onClose代码执行非法！
  auto p = _acceptor.removeClient(_peer);
  LOG_INFO << "handle of agent socket closed." << _peer;
}

ClientAgent::ClientAgent(LoopT *loop, TcpAcceptor &server, CodecI &codec)
    : _socket(loop), _acceptor(server), _ringbuffer(codec.size()),
      _codec(codec) {
  // _socket.setDefaultSize(1024, 1);
  _socket.writeCallback(std::bind(&ClientAgent::onWrite, this,
                                  std::placeholders::_1, std::placeholders::_2,
                                  std::placeholders::_3));
  _socket.readCallback(std::bind(&ClientAgent::onRead, this,
                                 std::placeholders::_1, std::placeholders::_2));
  _socket.shutdownCallback(
      std::bind(&ClientAgent::onShutdown, this, std::placeholders::_1));
  _socket.closeCallback(std::bind(&ClientAgent::onClose, this));
}

TcpI *ClientAgent::socket() const { return (TcpI *)&_socket; }

std::string ClientAgent::peer() {
  if (_peer.empty()) {
    _peer = nameOfPeer(AF_INET, &_socket);
  }
  return _peer;
}

// --

TcpAcceptor::TcpAcceptor(LoopT *loop, const struct sockaddr *addr,
                         CodecI &codec)
    : _socket(loop), _async(loop), _timer(loop), _loop(loop), _codec(codec) {
  const int backlog = 128;

  _socket.connectionCallback(
      std::bind(&TcpAcceptor::onConnection, this, std::placeholders::_1));
  _socket.shutdownCallback(
      std::bind(&TcpAcceptor::onShutdown, this, std::placeholders::_1));
  _socket.closeCallback(std::bind(&TcpAcceptor::onClose, this));

  int r = _socket.bind(addr, 0);
  LOG_IF_ERROR_EXIT(r);
  r = _socket.listen(backlog);
  LOG_IF_ERROR_EXIT(r);

  _name = nameOfSock(AF_INET, &_socket);
  LOG_INFO << "listen at: " << _name;

  _async.asyncCallback(std::bind(&TcpAcceptor::onAsync, this));

  _timer.timerCallback(std::bind(&TcpAcceptor::onTimer, this));
  _timer.start(1000, 100);
}

void TcpAcceptor::onConnection(int status) {
  if (status < 0) {
    LOG_IF_ERROR(status);
    return;
  }

  auto client = std::make_unique<ClientAgent>(_loop, *this, _codec);
  int r = _socket.accept(client->socket());
  if (r < 0) {
    LOG_IF_ERROR(r);
    return;
  }
  client->socket()->readStart();

  LOG_INFO << "accept connection from: " << client->peer();
  addClient(std::move(client));
}

void TcpAcceptor::onShutdown(int status) {
  if (status < 0) {
    LOG_IF_ERROR(status);
  }
  LOG_INFO << "listen socket shutdown.";
  _socket.close();
}

void TcpAcceptor::onClose() { LOG_INFO << "handle of listen socket closed."; }

void TcpAcceptor::onAsync() {
  Packet packet;
  while (downwardDequeue(packet)) {
    auto i = _clients.find(packet._peer);
    if (i == _clients.end()) {
      BufT b = packet.releaseBuf();
      freeBuf(b);
      continue;
    }

    BufT b = packet.releaseBuf();
    int r = i->second->socket()->write(&b, 1);
    if (r) {
      freeBuf(b);
      LOG_IF_ERROR(r);
    }
  }
  notifyHandler();
}

void TcpAcceptor::onTimer() {
  static bool stopped = false;
  if (_gangway._upward.size_approx() > 1000) {
    for (auto &c : _clients) {
      int r = c.second->socket()->readStop();
      LOG_IF_ERROR(r);
    }
    stopped = true;
    LOG_INFO << "clients read stoped.";
  }
  if (stopped && _gangway._upward.size_approx() < 10) {
    for (auto &c : _clients) {
      int r = c.second->socket()->readStart();
      LOG_IF_ERROR(r);
    }
    stopped = false;
    LOG_INFO << "clients read started.";
  }
}

void TcpAcceptor::notifyHandler() {
  if (_notifyTag == (int)NotifyTag::NT_NOTHING) {
    return;
  }

  if (_notifyTag == (int)NotifyTag::NT_CLOSE) {
    clientsShutdown();
    _socket.close();
    _timer.stop();
    _timer.close();
    _async.close();
    _notifyTag.store(int(NotifyTag::NT_NOTHING));
    return;
  }

  if (_notifyTag == (int)NotifyTag::NT_CLIENTS_SHUTDOWN) {
    clientsShutdown();
    _notifyTag.store(int(NotifyTag::NT_NOTHING));
    return;
  }
}

void TcpAcceptor::clientsShutdown() {
  for (auto &c : _clients) {
    int r = c.second->socket()->readStop();
    LOG_IF_ERROR(r);
    r = c.second->socket()->shutdown();
    if (r) {
      c.second->socket()->close();
      LOG_IF_ERROR(r);
    }
  }
}

void TcpAcceptor::addClient(std::unique_ptr<ClientAgent> &&client) {
  std::string n = client->peer();
  auto i = _clients.insert({n, std::forward<decltype(client)>(client)});
  if (!i.second) {
    LOG_CRIT << "client agent has a same name: " << n;
  }
  LOG_INFO << "add client: " << n;
}

std::unique_ptr<ClientAgent>
TcpAcceptor::removeClient(const std::string &name) {
  auto i = _clients.find(name);
  if (i == _clients.end()) {
    LOG_CRIT << "can't find the client: " << name;
    return std::unique_ptr<ClientAgent>();
  }
  auto p = std::move(i->second);

  int num = _clients.erase(name);
  LOG_INFO << "remove " << num << " client: " << name;

  return p;
}

bool TcpAcceptor::upwardEnqueue(Packet &&packet) {
  return _gangway._upward.enqueue(std::forward<Packet>(packet));
}

bool TcpAcceptor::upwardDequeue(Packet &packet) {
  return _gangway._upward.wait_dequeue_timed(packet,
                                             std::chrono::milliseconds(500));
}

int TcpAcceptor::downwardEnqueue(const char *name, const char *p, size_t len) {
  BufT b = _codec.encode(p, len);
  if (!b.base || !b.len) {
    assert(false);
    return 0;
  }
  Packet packet(name, b);
  _gangway._downward.enqueue(std::move(packet));

  int r = _async.send();
  LOG_IF_ERROR(r);
  return r;
}

bool TcpAcceptor::downwardDequeue(Packet &packet) {
  return _gangway._downward.try_dequeue(packet);
}

int TcpAcceptor::notify(int tag) {
  _notifyTag.store(tag);
  int r = _async.send();
  LOG_IF_ERROR(r);
  return r;
}

// --

TcpAcceptor *g_acceptor;
Business *g_business;

int tcp_server() {
  Codec2 codec('|');

  // 建立线程池，线程个数1
  uvp::ThreadPool pool(1);

  auto loop = LoopT::defaultLoop();
  // auto loop = std::make_unique<LoopT>();
  struct sockaddr_in addr;
  uv_ip4_addr("0", 7001, &addr);
  TcpAcceptor acceptor(loop.get(), (const struct sockaddr *)&addr, codec);
  g_acceptor = &acceptor;

  Business bness("business");
  bness.bind(&acceptor);
  g_business = &bness;
  pool.execute(std::ref(bness));

  int r = loop->run(UV_RUN_DEFAULT);
  LOG_IF_ERROR(r);
  r = loop->close();
  LOG_IF_ERROR(r);

  pool.stop();
  pool.join_all();

  return r;
}
