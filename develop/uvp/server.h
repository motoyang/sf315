#pragma once

#include <string>
#include <list>
#include <vector>
#include <unordered_map>

#include <ringbuffer.hpp>
#include <uv.hpp>
#include <gangway.hpp>

// --

class TcpAcceptor;
class ClientAgent {
  TcpT _socket;
  std::string _peer;
  TcpAcceptor& _acceptor;
   
  CodecI& _codec;

  void makeup(const char* p, size_t len);

  void onRead(ssize_t nread, const BufT* buf);
  void onWrite(int status, BufT bufs[], int nbufs);
  void onShutdown(int status);
  void onClose();

public:
  ClientAgent(LoopI* loop, TcpAcceptor& server, CodecI& codec);

  void write(int index);
  TcpI* socket() const;
  std::string peer();
};

// --

class TcpAcceptor {
  TcpT _socket;
  AsyncT _async;
  TimerT _timer;

  LoopI* _loop;
  std::unordered_map<std::string, std::unique_ptr<ClientAgent>> _clients;
  
  CodecI& _codec;
  Gangway _gangway;
  
  std::string _name;
  std::atomic<int> _notifyTag {0};

  void notifyHandler();
  void clientsShutdown();

  void onConnection(int status);
  void onShutdown(int status);
  void onClose();

  void onAsync();
  void onTimer();

public:
  enum class NotifyTag {
    NT_NOTHING = 0,
    NT_CLOSE,
    NT_CLIENTS_SHUTDOWN
  };

  TcpAcceptor(LoopI *loop, const struct sockaddr *addr, CodecI& codec);
  void addClient(std::unique_ptr<ClientAgent>&& client);
  std::unique_ptr<ClientAgent> removeClient(const std::string& name);
  bool upwardEnqueue(Packet&& packet);
  bool upwardDequeue(Packet& packet);
  int downwardEnqueue(const char* name, const char* p, size_t len);
  bool downwardDequeue(Packet& packet);
  int notify(int tag);
};

// --

int tcp_server();