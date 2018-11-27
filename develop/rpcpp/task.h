#pragma once

#include "concurrentqueue.h"
#include "objects.h"
#include "node.h"

// --

namespace rpcpp {

// --

class Task : public Object {
public:
  Task(const std::string &n) : Object(n) {}
};

// --

template <typename T> class PassiveTask : public Task {
  std::unique_ptr<T> _node;

public:
  PassiveTask(const std::string &name, std::unique_ptr<T> &&node)
      : Task(name), _node(std::forward<decltype(node)>(node)) {}

  int operator()() {
    while (_node->isRunning()) {
      _node->transact();
    }
    return 0;
  }

  virtual void close() override { _node->close(); }
};

// --

template<typename T> using PullTask = PassiveTask<PullNode<T>>;
template<typename T> using SubscribeTask = PassiveTask<SubscribeNode<T>>;
template<typename T> using ReplyTask = PassiveTask<ReplyNode<T>>;
template<typename T> using ResponseTask = PassiveTask<ResponseNode<T>>;

// --

template <typename T> class ActiveTask : public Task {
public:
  using ChunkFun = std::function<void(T *)>;

private:
  std::unique_ptr<T> _node;
  moodycamel::ConcurrentQueue<ChunkFun> _que;

public:
  ActiveTask(const std::string &name, std::unique_ptr<T> &&n)
      : Task(name), _node(std::forward<decltype(n)>(n)) {}

  int operator()(int ms) {
    while (_node->isRunning()) {
      ChunkFun f;
      while (_que.try_dequeue(f)) {
        f(_node.get());
      }
      if (ms > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
      }
    }
    return 0;
  }

  bool addChunk(ChunkFun &&f) {
    return _que.enqueue(std::forward<ChunkFun>(f));
  }

  T *node() const { return _node.get(); }

  virtual void close() override { _node->close(); }
};

// --

template<typename T> using PushTask = ActiveTask<PushNode<T>>;
template<typename T> using PublishTask = ActiveTask<PublishNode<T>>;
template<typename T> using RequestTask = ActiveTask<RequestNode<T>>;

// --

template <typename T> class MixedTask : public Task {
public:
  using ChunkFun = std::function<void(T *)>;

private:
  std::unique_ptr<T> _node;
  moodycamel::ConcurrentQueue<ChunkFun> _que;

public:
  MixedTask(const std::string &name, std::unique_ptr<T> &&n)
      : Task(name), _node(std::forward<decltype(n)>(n)) {}

  int operator()(int ms) {
    while (_node->isRunning()) {
      ChunkFun f;
      while (_que.try_dequeue(f)) {
        f(_node.get());
      }

      _node->transact();

      if (ms > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
      }
    }
    return 0;
  }

  bool addChunk(ChunkFun &&f) {
    return _que.enqueue(std::forward<ChunkFun>(f));
  }

  T *node() const { return _node.get(); }

  virtual void close() override { _node->close(); }
};

// --

template<typename T> using PairTask = MixedTask<PairNode<T>>;
template<typename T> using BusTask = MixedTask<BusNode<T>>;
template<typename T> using SurveyTask = MixedTask<SurveyNode<T>>;

} // namespace rpcpp
