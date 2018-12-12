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

      // 如果上传队列满了，就等待5ms再次上传
      // while (!_acceptor.gangway()._upward.try_enqueue(std::move(packet))) {
      //   std::this_thread::sleep_for(std::chrono::milliseconds(5));
      // }
      _acceptor.upwardEnqueue(std::move(packet));
    }
  } while (remain > 0);
}

void ClientAgent::onRead(ssize_t nread, const BufT *buf) {
  if (nread < 0) {
    if (UV_EOF == nread || UV_ECONNRESET == nread)
     {
      // FIXME!!! closed() should be called directly?
      _socket.close();
      // int r = _socket.shutdown();
      // if (r) {
      //   LOG_IF_ERROR(r);
      //   _socket.close();
      // }
      return;
    }
    LOG_IF_ERROR(nread);
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
    : _socket(loop), _async(loop), _loop(loop), _codec(codec) {
  const int backlog = 128;

  _socket.connectionCallback(
      std::bind(&TcpAcceptor::onConnection, this, std::placeholders::_1));
  _socket.shutdownCallback(
      std::bind(&TcpAcceptor::onShutdown, this, std::placeholders::_1));

  int r = _socket.bind(addr, 0);
  LOG_IF_ERROR_EXIT(r);
  r = _socket.listen(backlog);
  LOG_IF_ERROR_EXIT(r);

  _name = nameOfSock(AF_INET, &_socket);
  LOG_INFO << "listen at: " << _name;

  _async.asyncCallback(std::bind(&TcpAcceptor::onAsync, this));
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
  // while (_gangway._downward.try_dequeue(packet)) {
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

void TcpAcceptor::upwardEnqueue(Packet &&packet) {
  // 如果队列满了，就等待5ms再次上传
  while (!_gangway._upward.enqueue(std::forward<Packet>(packet))) {
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
}

bool TcpAcceptor::upwardDequeue(Packet &packet) {
  return _gangway._upward.wait_dequeue_timed(packet,
                                             std::chrono::milliseconds(500));
}

int TcpAcceptor::downwardEnqueue(const char *name, const char *p, size_t len) {
  BufT b = _codec.encode(p, len);
  Packet packet(name, b);

  // 如果队列满了，就等待5ms再次上传
  while (!_gangway._downward.enqueue(std::move(packet))) {
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  int r = _async.send();
  LOG_IF_ERROR(r);
  return r;
}

bool TcpAcceptor::downwardDequeue(Packet &packet) {
  return _gangway._downward.try_dequeue(packet);
}

// --

int tcp_server() {
  Codec2 codec('|');

  // 建立线程池，线程个数1
  uvp::ThreadPool pool(1);

  auto loop = LoopT::defaultLoop();
  // auto loop = std::make_unique<LoopT>();
  struct sockaddr_in addr;
  uv_ip4_addr("0", 7001, &addr);
  TcpAcceptor acceptor(loop.get(), (const struct sockaddr *)&addr, codec);

  Business bness("business");
  bness.bind(&acceptor);
  pool.execute(std::ref(bness));

  int r = loop->run(UV_RUN_DEFAULT);
  LOG_IF_ERROR(r);
  r = loop->close();
  LOG_IF_ERROR(r);

  pool.join_all();

  return r;
}
