#include <iostream>
#include <atomic>

#include <utilites.hpp>
#include <misc.hpp>
#include <uv.hpp>
#include <req.hpp>

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
      _listenor.gangway()._upward.enqueue(std::move(packet));
    }
  } while (remain > 0);
}

void ClientAgent::onRead(ssize_t nread, const BufT *buf) {
  if (nread < 0) {
    if (UV_EOF == nread || UV_ECONNRESET == nread) {
      // FIXME!!! closed() should be called directly?
      int r = _socket.shutdown();
      if (r) {
        LOG_IF_ERROR(r);
        _socket.close();
      }
      return;
    }
    LOG_IF_ERROR(nread);
  }

  // 整理包，并进一步处理包
  makeup(buf->base, nread);
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
  auto p = _listenor.removeClient(_peer);
  LOG_INFO << "handle of agent socket closed." << _peer;
}

void ClientAgent::write(int index) {
  index %= _msgList.size();
  for (int i = 0; i < _msgList.size(); ++i) {
    if (index == _msgList.size())
      index = 0;
    auto &msg = _msgList[index++];
    BufT b = _codec.encode(msg.c_str(), msg.length());
    // BufT b = copyToBuf(msg.data(), msg.length());
    int r = _socket.write(&b, 1);
    if (r) {
      freeBuf(b);
      LOG_IF_ERROR(r);
    }
  }
  // _socket.readStart();
}

ClientAgent::ClientAgent(LoopT *loop, TcpServer &server)
    : _socket(loop), _listenor(server), _ringbuffer(512), _codec('|') {
  // _msgList.push_back("4|test");
  // _msgList.push_back("5|Hello");
  // _msgList.push_back("6|World!");
  // _msgList.push_back("20|abcdefghij0123456789");
  _msgList.push_back("test");
  _msgList.push_back("Hello");
  _msgList.push_back("World!");
  _msgList.push_back("test_abc");
  _msgList.push_back("Hello, everyon");
  _msgList.push_back("World! Heart!");
  _msgList.push_back("abcdefghij0123456789");
  _socket.writeCallback(std::bind(&ClientAgent::onWrite, this,
                                  std::placeholders::_1, std::placeholders::_2,
                                  std::placeholders::_3));
  _socket.readCallback(std::bind(&ClientAgent::onRead, this,
                                 std::placeholders::_1, std::placeholders::_2));
  _socket.shutdownCallback(
      std::bind(&ClientAgent::onShutdown, this, std::placeholders::_1));
}

TcpI *ClientAgent::socket() const { return (TcpI *)&_socket; }

std::string ClientAgent::peer() {
  if (_peer.empty()) {
    _peer = nameOfPeer(AF_INET, &_socket);
  }
  return _peer;
}

// --

TcpServer::TcpServer(LoopT *loop, Gangway &way, const struct sockaddr *addr)
    : _socket(loop), _timer(loop), _async(loop), _loop(loop), _gangway(way) {
  const int backlog = 128;
  std::srand(std::time(nullptr));

  _socket.connectionCallback(
      std::bind(&TcpServer::onConnection, this, std::placeholders::_1));
  _socket.shutdownCallback(
      std::bind(&TcpServer::onShutdown, this, std::placeholders::_1));

  int r = _socket.bind(addr, 0);
  LOG_IF_ERROR_EXIT(r);
  r = _socket.listen(backlog);
  LOG_IF_ERROR_EXIT(r);

  _name = nameOfSock(AF_INET, &_socket);
  LOG_INFO << "listen at: " << _name;

  _timer.timerCallback(std::bind(&TcpServer::onTimer, this));
  // _timer.start(1000, 1000);

  _async.asyncCallback(std::bind(&TcpServer::onAsync, this));
}

void TcpServer::onConnection(int status) {
  if (status < 0) {
    LOG_IF_ERROR(status);
  }

  auto client = std::make_unique<ClientAgent>(_loop, *this);
  int r = _socket.accept(client->socket());
  if (r < 0) {
    LOG_IF_ERROR(r);
    return;
  }
  client->socket()->readStart();

  LOG_INFO << "accept connection from: " << client->peer();
  addClient(std::move(client));
}

void TcpServer::onShutdown(int status) {
  if (status < 0) {
    LOG_IF_ERROR(status);
  }
  LOG_INFO << "listen socket shutdown.";
  _socket.close();
}

void TcpServer::onClose() { LOG_INFO << "handle of listen socket closed."; }

void TcpServer::onTimer() {
  int random_variable = std::rand();
  for (auto &i : _clients) {
    i.second->write(random_variable);
  }
}

void TcpServer::onAsync() {
  Packet packet;
  while (_gangway._downward.try_dequeue(packet)) {
    auto i = _clients.find(packet._peer);
    if (i == _clients.end()) {
      continue;
    }
    BufT b = packet._buf;
    packet._buf.len = 0;
    packet._buf.base = nullptr;
    
    int r = i->second->socket()->write(&b, 1);
    if (r) {
      freeBuf(b);
    }
  }
}
void TcpServer::addClient(std::unique_ptr<ClientAgent> &&client) {
  std::string n = client->peer();
  auto i = _clients.insert({n, std::forward<decltype(client)>(client)});
  if (!i.second) {
    LOG_CRIT << "client agent has a same name: " << n;
  }
  LOG_INFO << "add client: " << n;
}

std::unique_ptr<ClientAgent> TcpServer::removeClient(const std::string &name) {
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

Gangway &TcpServer::gangway() { return _gangway; }

AsyncI *TcpServer::async() { return &_async; }

// --

int tcp_server(LoopT *loop) {
  Gangway gway;

  Business bness("business", gway);
  bness.start(loop);

  struct sockaddr_in addr;
  uv_ip4_addr("0", 7001, &addr);

  TcpServer server(loop, gway, (const struct sockaddr *)&addr);
  bness.tcp(&server);

  return loop->run(UV_RUN_DEFAULT);
}
