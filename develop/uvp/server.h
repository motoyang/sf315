#pragma once

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
   
  RingBuffer _ringbuffer;
  CodecI& _codec;
  void makeup(const char* p, size_t len);

  void onRead(ssize_t nread, const BufT* buf);
  void onWrite(int status, BufT bufs[], int nbufs);
  void onShutdown(int status);
  void onClose();

public:
  ClientAgent(LoopT* loop, TcpAcceptor& server, CodecI& codec);

  void write(int index);
  TcpI* socket() const;
  std::string peer();
};

// --

class TcpAcceptor {
  TcpT _socket;
  AsyncT _async;
  LoopT* _loop;
  CodecI& _codec;
  std::string _name;
  std::unordered_map<std::string, std::unique_ptr<ClientAgent>> _clients;
  Gangway _gangway;

  void onConnection(int status);
  void onShutdown(int status);
  void onClose();

  void onAsync();

public:
  TcpAcceptor(LoopT *loop, const struct sockaddr *addr, CodecI& codec);
  void addClient(std::unique_ptr<ClientAgent>&& client);
  std::unique_ptr<ClientAgent> removeClient(const std::string& name);
  void upwardEnqueue(Packet&& packet);
  bool upwardDequeue(Packet& packet);
  // int downwardEnqueue(Packet&& packet);
  int downwardEnqueue(const char* name, const char* p, size_t len);
  bool downwardDequeue(Packet& packet);
};

// --

int tcp_server();