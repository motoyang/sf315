#pragma once

#include <string>
#include <memory>

#include "concurrentqueue.h"
#include "socket.h"
/*
template <typename S, typename R, int Q> class Worker {
  std::shared_ptr<Socket> _sock;
  std::shared_ptr<moodycamel::ConcurrentQueue<T>> _jobsQue;
  std::shared_ptr<moodycamel::ConcurrentQueue<std::string>> _resultsQue;

public:
  Worker(Socket::OpenFun &&f)
    : _sock(std::make_share<Socket>(f))
    , _jobsQue(std::make_share<moodycamel::ConcurrentQueue<T>>())
    , _resultsQue(std::make_share<moodycamel::ConcurrentQueue<std::string>>()) {}
  
  int operator()();
  bool addJob(T &&job);
  bool getResults(std::string &result);
};
*/