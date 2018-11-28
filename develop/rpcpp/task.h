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
  Task(const Task& t) : Object(t._name){
    assert(false);
  }
  Task(Task&& t) :Object(std::forward<Task>(t)._name){
    LOG_INFO << "move task.";
  }
};

// --

template <typename T> class InteractiveTask : public Task {
public:
  using ChunkFun = std::function<void(T *)>;

private:
  std::unique_ptr<T> _node;
  moodycamel::ConcurrentQueue<ChunkFun> _que;

public:
  InteractiveTask(const std::string &name, std::unique_ptr<T> &&n)
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

  bool load(ChunkFun &&f) { return _que.enqueue(std::forward<ChunkFun>(f)); }

  virtual void close() override { _node->close(); }
};

// --

template <typename T> using RequestTask = InteractiveTask<RequestNode<T>>;

// --

template <typename T> class SemiInteractiveTask : public Task {
  std::unique_ptr<T> _node;
  moodycamel::ConcurrentQueue<std::string> _que;

public:
  using TagType = typename T::TagType;

  SemiInteractiveTask(const std::string &name, std::unique_ptr<T> &&n)
      : Task(name), _node(std::forward<decltype(n)>(n)) {}

  int operator()(int ms) {
    while (_node->isRunning()) {
      if (ms > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
      }

      std::string s;
      if (!_que.try_dequeue(s)) {
        continue;
      }

      _node->send(s.data(), s.size());
      while (0 == _node->transact())
        ;
    }
    return 0;
  }

  template <typename... Args> bool post(TagType const &tag, Args &&... args) {
    std::stringstream ss;
    msgpack::pack(ss, tag);
    ((msgpack::pack(ss, std::forward<Args>(args)), ...));
    return _que.enqueue(ss.str());
  }

  virtual void close() override { _node->close(); }
};

// --

template <typename T> using SurveyTask = SemiInteractiveTask<SurveyNode<T>>;

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

template <typename T> using PullTask = PassiveTask<PullNode<T>>;
template <typename T> using SubscribeTask = PassiveTask<SubscribeNode<T>>;
template <typename T> using ReplyTask = PassiveTask<ReplyNode<T>>;

// --

template <typename T> class ActiveTask : public Task {
  std::unique_ptr<T> _node;
  moodycamel::ConcurrentQueue<std::string> _que;

public:
  using TagType = typename T::TagType;

  ActiveTask(const std::string &name, std::unique_ptr<T> &&n)
      : Task(name), _node(std::forward<decltype(n)>(n)) {}

  int operator()(int ms) {
    while (_node->isRunning()) {
      std::string s;
      while (_que.try_dequeue(s)) {
        _node->send(s.data(), s.size());
      }

      if (ms > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
      }
    }
    return 0;
  }

  template <typename... Args> bool post(TagType const &tag, Args &&... args) {
    std::stringstream ss;
    msgpack::pack(ss, tag);
    ((msgpack::pack(ss, std::forward<Args>(args)), ...));
    return _que.enqueue(ss.str());
  }

  virtual void close() override { _node->close(); }
};

// --

template <typename T> using PushTask = ActiveTask<PushNode<T>>;
template <typename T> using PublishTask = ActiveTask<PublishNode<T>>;

// --

template <typename T> class MixedTask : public Task {
  std::unique_ptr<T> _node;
  moodycamel::ConcurrentQueue<std::string> _que;

public:
  using TagType = typename T::TagType;

  MixedTask(const std::string &name, std::unique_ptr<T> &&n)
      : Task(name), _node(std::forward<decltype(n)>(n)) {}

  int operator()(int ms) {
    while (_node->isRunning()) {
      // std::cout << "_que.size(): " << _que.size_approx() << std::endl;
      // std::cout << "this2 = " << this << ", name: " << _name << std::endl;
      std::string s;
      if (_que.try_dequeue(s)) {
        _node->send(s.data(), s.size());
      }

      _node->transact();

      if (ms > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
      }
    }
    return 0;
  }

  template <typename... Args> bool post(TagType const &tag, Args &&... args) {
    std::stringstream ss;
    msgpack::pack(ss, tag);
    ((msgpack::pack(ss, std::forward<Args>(args)), ...));
    bool b = _que.enqueue(ss.str());
    // std::cout << "b = " << b << ", size: " << _que.size_approx() << std::endl;
    // std::cout << "this1 = " << this << std::endl;
    return b;
  }

  virtual void close() override { _node->close(); }
};

// --

template <typename T> using PairTask = MixedTask<PairNode<T>>;
template <typename T> using BusTask = MixedTask<BusNode<T>>;
template <typename T> using ResponseTask = MixedTask<ResponseNode<T>>;

// --

} // namespace rpcpp
