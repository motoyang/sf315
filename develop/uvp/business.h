#pragma once

#include <gangway.hpp>
#include <resolver.h>
// --

class TcpAcceptor;
class Business {
  TcpAcceptor* _tcp;
  std::unique_ptr<uvp::Replier<int>> _replier;
  std::unique_ptr<uvp::Resolver<int>> _resolver;

  std::string _name;
  std::atomic<bool> _running{true};

  void doSomething(const Packet &p);

public:
  Business(const std::string &name);
  void bind(TcpAcceptor* tcp);

  void operator()();
  void stop();
};
