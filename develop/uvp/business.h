#pragma once

#include <gangway.hpp>

// --

class TcpServer;
class Business {
  WorkT _work;
  Gangway &_gangway;
  TcpServer* _tcp;

  Codec _codec;
  std::string _name;
  std::atomic<bool> _running{true};

  void workCallback();
  void afterWorkCallback(int status);
  void doSomething(Packet &&p);

public:
  Business(const std::string &name, Gangway &way);
  int start(LoopT *from);
  void stop();
  void tcp(TcpServer* tcp);
};
