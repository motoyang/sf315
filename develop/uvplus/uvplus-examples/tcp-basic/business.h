#pragma once

#include <uvp.hpp>
#include <uvplus.hpp>

// --

class Business {
  uvplus::TcpAcceptor<uvplus::Codec2>* _tcp;
  std::unique_ptr<uvplus::Replier<int>> _replier;
  std::unique_ptr<uvplus::Resolver<int>> _resolver;

  std::string _name;
  std::atomic<bool> _running{true};

  uvplus::ThreadPool _pool;

  void doSomething(const uvplus::Packet &p);

public:
  Business(const std::string &name);
  void bind(uvplus::TcpAcceptor<uvplus::Codec2>* tcp);

  void operator()();
  void stop();
};
