#pragma once

#include <list>
#include <vector>
#include <unordered_map>

#include <uv.hpp>

// --

class Packet;
class TcpServer;
class ClientAgent {
  TcpT _socket;
  std::string _peer;
  TcpServer& _listenor;
  std::vector<std::string> _msgList;
   
  RingBuffer _ringbuffer;
  Codec _codec;
  void makeup(const char* p, size_t len);

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
class Gangway;
class TcpServer {
  TcpT _socket;
  TimerT _timer;
  LoopT* _loop;
  std::string _name;
  std::unordered_map<std::string, std::unique_ptr<ClientAgent>> _clients;
  Gangway & _gangway;

  void onConnection(int status);
  void onShutdown(int status);
  void onClose();

  void onTimer();

public:
  TcpServer(LoopT *loop, Gangway& way, const struct sockaddr *addr);
  void addClient(std::unique_ptr<ClientAgent>&& client);
  std::unique_ptr<ClientAgent> removeClient(const std::string& name);
  Gangway& gangway();
};

// --

int tcp_server(LoopT *loop);