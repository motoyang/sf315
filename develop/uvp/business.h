#pragma once

#include <gangway.hpp>

// --

class TcpAcceptor;
class Business {
  TcpAcceptor* _tcp;

  std::string _name;
  std::atomic<bool> _running{true};

  void doSomething(const Packet &p);

public:
  Business(const std::string &name);
  void bind(TcpAcceptor* tcp);

  void operator()();
  void stop();
};
