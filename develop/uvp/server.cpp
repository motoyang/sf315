#include <iostream>

#include <uv.h>
#include <utilites.hpp>
#include <misc.hpp>
#include <uv.hpp>

#include "server.h"

// --

// 1st paramater: AF_INET or AF_INET6 for ipv6
std::string nameOfPeer(int af, TcpI *tcp) {
  sockaddr addr;
  sockaddr_in *paddr_in = (sockaddr_in *)&addr;

  int len = sizeof(addr);
  int r = tcp->getpeername(&addr, &len);
  LOG_IF_ERROR(r);

  char name[INET6_ADDRSTRLEN] = {0};
  r = uv_inet_ntop(AF_INET, (const void *)&paddr_in->sin_addr, name,
                   sizeof(name));
  LOG_IF_ERROR(r);
  int port = ntohs(paddr_in->sin_port);

  std::stringstream ss;
  ss << name << ":" << port;
  return ss.str();
}

// 1st paramater: AF_INET or AF_INET6 for ipv6
std::string nameOfSock(int af, TcpI *tcp) {
  sockaddr addr;
  sockaddr_in *paddr_in = (sockaddr_in *)&addr;

  int len = sizeof(addr);
  int r = tcp->getsockname(&addr, &len);
  LOG_IF_ERROR(r);

  char name[INET6_ADDRSTRLEN] = {0};
  r = uv_inet_ntop(AF_INET, (const void *)&paddr_in->sin_addr, name,
                   sizeof(name));
  LOG_IF_ERROR(r);
  int port = ntohs(paddr_in->sin_port);

  std::stringstream ss;
  ss << name << ":" << port;
  return ss.str();
}

// --

void ClientAgent::onRead(ssize_t nread, const BufT *buf) {
  if (nread < 0) {
    if (nread == UV_EOF) {
      int r = _socket.shutdown();
      LOG_IF_ERROR(r);
      return;
    }
    LOG_IF_ERROR(nread);
  }
  // _solver.doBussiness(buf->base, nread);
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
    return;
  }
  LOG_INFO << "agent socket shutdown.";

  // TcpServer &server = _server;
  _socket.close([this]() {
    _server.removeClient(_peer);
    LOG_INFO << "handle of agent socket closed.";
  });
}

void ClientAgent::write() {
  for (auto &msg : _msgList) {
    BufT b = copyToBuf(msg.data(), msg.length());
    int r = _socket.write(&b, 1);
    LOG_IF_ERROR(r);
  }
}

ClientAgent::ClientAgent(LoopT *loop, TcpServer &server)
    : _socket(loop), _server(server) {
  _msgList.push_back("4|test");
  _msgList.push_back("5|Hello");
  _msgList.push_back("6|World!");
  _msgList.push_back("20|abcdefghij0123456789");

  _socket.writeCallback(std::bind(&ClientAgent::onWrite, this,
                                  std::placeholders::_1, std::placeholders::_2,
                                  std::placeholders::_3));
  _socket.readCallback(std::bind(&ClientAgent::onRead, this,
                                 std::placeholders::_1, std::placeholders::_2));
  _socket.shutdownCallback(
      std::bind(&ClientAgent::onShutdown, this, std::placeholders::_1));
}

TcpI *ClientAgent::socket() const { return (TcpI *)&_socket; }

std::string ClientAgent::peer() const { return _peer; }

void ClientAgent::peer(const std::string &peer) { _peer = peer; }

// --

TcpServer::TcpServer(LoopT *loop, const struct sockaddr *addr)
    : _socket(loop), _timer(loop), _loop(loop) {
  const int backlog = 128;

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
  std::string peer = nameOfPeer(AF_INET, client->socket());
  LOG_INFO << "accept connection from: " << peer;

  client->peer(peer);
  client->write();
  r = client->socket()->shutdown();
  LOG_IF_ERROR(r);

  addClient(std::move(client));
}

void TcpServer::onShutdown(int status) {
  if (status < 0) {
    LOG_IF_ERROR(status);
    return;
  }
  LOG_INFO << "listen socket shutdown.";
  _socket.close([]() { LOG_INFO << "handle of listen socket closed."; });
}

void TcpServer::addClient(std::unique_ptr<ClientAgent> &&client) {
  while (_cbak.size() > 0) {
    auto c = std::move(_cbak.front());      
    _cbak.pop_front();

    LOG_INFO << "client agent removed from list: " << c->peer();
  }

  std::string n = client->peer();
  auto i = _clients.insert({n, std::forward<decltype(client)>(client)});
  if (!i.second) {
    LOG_CRIT << "client agent has a same name: " << n;
  }
  LOG_INFO << "add client: " << n;
}

void TcpServer::removeClient(const std::string &name) {
  auto i = _clients.find(name);
  if (i == _clients.end()) {
    LOG_CRIT << "can't find the client: " << name;
    return;
  }
  _cbak.push_back(std::move(i->second));

  int num = _clients.erase(name);
  LOG_INFO << "remove " << num << " client: " << name;
}

// --

int tcp_server(LoopT *loop) {
  struct sockaddr_in addr;
  uv_ip4_addr("0", 7001, &addr);

  TcpServer server(loop, (const struct sockaddr *)&addr);
  return loop->run(UV_RUN_DEFAULT);
}
