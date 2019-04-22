#pragma once

#include <uvp.hpp>
#include <uvplus.hpp>

// --

class Business {
  SecureAcceptor* _tcp;

  std::string _name;
  std::atomic<bool> _running{true};

  uvplus::ThreadPool _pool;

  void doSomething(const uvplus::Packet &p);

public:
  Business(const std::string &name);
  void bind(SecureAcceptor* tcp);

  void operator()();
  void stop();
};
