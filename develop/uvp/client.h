#pragma once

#include <ringbuffer.hpp>

// --

class ParketSolver {
  RingBuffer _buf;
  std::string _msg;

  char *parse(char *msg, size_t msg_size, RingBuffer *buf);

public:
  ParketSolver(size_t size);

  size_t size() const { return _buf.capacity(); };
  void doBussiness(const char *p, size_t len);
};

// --

class TcpClient {
  TcpT _socket;
  ParketSolver _solver;

  void onRead(ssize_t nread, const BufT *buf);
  void onConnect(int status);
  void onShutdown(int status);

public:
  TcpClient(LoopT *loop, const struct sockaddr *addr);
};

// --

int tcp_client(LoopT *loop);
