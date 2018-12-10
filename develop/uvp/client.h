#pragma once

#include <ringbuffer.hpp>
#include <uv.hpp>
#include <gangway.hpp>

// --

// --

class TcpClient {
  TcpT _socket;
  std::string _name;

  TimerT _timer;
  std::vector<std::string> _msgList;

  RingBuffer _ringbuffer;
  Codec _codec;

  void onRead(ssize_t nread, const BufT *buf);
  void onWrite(int status, BufT bufs[], int nbufs);
  void onConnect(int status);
  void onShutdown(int status);
  void onClose();
  void onTimer();

  void doBussiness(const char* p, size_t len);
public:
  TcpClient(LoopT *loop, const struct sockaddr *addr);
};

// --

int tcp_client(LoopT *loop);
