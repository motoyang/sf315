#pragma once

#include <list>
#include <unordered_map>

#include <uv.hpp>

// --

class ParcketCreator {

public:
  ParcketCreator();
};

class TcpServer;
class ClientAgent {
  TcpT _socket;
  std::string _peer;
  TcpServer& _server;
  std::list<std::string> _msgList;

  void onRead(ssize_t nread, const BufT* buf);
  void onWrite(int status, BufT bufs[], int nbufs);
  void onShutdown(int status);

public:
  ClientAgent(LoopT* loop, TcpServer& server);

  void write();
  TcpI* socket() const;
  std::string peer() const;
  void peer(const std::string& peer);
};

// --

class TcpServer {
  TcpT _socket;
  std::unordered_map<std::string, std::unique_ptr<ClientAgent>> _clients;
  TimerT _timer;
  LoopT* _loop;
  std::string _name;
  std::list<std::unique_ptr<ClientAgent>> _cbak;

  void onConnection(int status);
  void onShutdown(int status);

public:
  TcpServer(LoopT *loop, const struct sockaddr *addr);
  void addClient(std::unique_ptr<ClientAgent>&& client);
  void removeClient(const std::string& name);
};

// --

int tcp_server(LoopT *loop);