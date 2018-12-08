#pragma once

#include <list>
#include <vector>
#include <unordered_map>

#include <uv.hpp>

#include "client.h"

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
  std::vector<std::string> _msgList;
  ParketSolver _solver;

  void onRead(ssize_t nread, const BufT* buf);
  void onWrite(int status, BufT bufs[], int nbufs);
  void onShutdown(int status);
  void onClose();

public:
  ClientAgent(LoopT* loop, TcpServer& server);

  void write(int index);
  TcpI* socket() const;
  std::string peer() const;
  void peer(const std::string& peer);
};

// --

class TcpServer {
  TcpT _socket;
  TimerT _timer;
  LoopT* _loop;
  std::string _name;
  std::unordered_map<std::string, std::unique_ptr<ClientAgent>> _clients;

  void onConnection(int status);
  void onShutdown(int status);
  void onClose();

  void onTimer();

public:
  TcpServer(LoopT *loop, const struct sockaddr *addr);
  void addClient(std::unique_ptr<ClientAgent>&& client);
  std::unique_ptr<ClientAgent> removeClient(const std::string& name);
};

// --

int tcp_server(LoopT *loop);