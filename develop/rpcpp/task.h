#pragma once

#include "concurrentqueue.h"
#include "objects.h"
#include "node.h"

// --

namespace rpcpp {

// --

template <typename T> class Task : public Object {
protected:
  std::unique_ptr<T> _node;

public:
  Task(const std::string &n, std::unique_ptr<T> &&node)
      : Object(n), _node(std::forward<decltype(node)>(node)) {}

  virtual void close() override { _node->close(); }
};

// --

template <typename T> class InteractiveTask : public Task<T> {
public:
  using ChunkFun = std::function<void(T *)>;

protected:
  moodycamel::ConcurrentQueue<ChunkFun> _que;

public:
  InteractiveTask(const std::string &name, std::unique_ptr<T> &&n)
      : Task<T>(name, std::forward<decltype(n)>(n)) {}

  int operator()(int ms) {
    while (Task<T>::_node->isRunning()) {
      ChunkFun f;
      while (_que.try_dequeue(f)) {
        f(Task<T>::_node.get());
      }
      if (ms > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
      }
    }
    return 0;
  }

  bool load(ChunkFun &&f) { return _que.enqueue(std::forward<ChunkFun>(f)); }
};

// --

template <typename T> using RequestTask = InteractiveTask<RequestNode<T>>;

// --

template <typename T> class SemiInteractiveTask : public Task<T> {
protected:
  moodycamel::ConcurrentQueue<std::string> _que;

public:
  using TagType = typename T::TagType;

  SemiInteractiveTask(const std::string &name, std::unique_ptr<T> &&n)
      : Task<T>(name, std::forward<decltype(n)>(n)) {}

  int operator()(int ms) {
    while (Task<T>::_node->isRunning()) {
      if (ms > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
      }

      std::string s;
      if (!_que.try_dequeue(s)) {
        continue;
      }

      Task<T>::_node->send(s.data(), s.size());
      while (0 == Task<T>::_node->transact())
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
};

// --

template <typename T> using SurveyTask = SemiInteractiveTask<SurveyNode<T>>;

// --

template <typename T> class PassiveTask : public Task<T> {
public:
  PassiveTask(const std::string &name, std::unique_ptr<T> &&node)
      : Task<T>(name, std::forward<decltype(node)>(node)) {}

  int operator()() {
    while (Task<T>::_node->isRunning()) {
      Task<T>::_node->transact();
    }
    return 0;
  }
};

// --

template <typename T> using PullTask = PassiveTask<PullNode<T>>;
template <typename T> using SubscribeTask = PassiveTask<SubscribeNode<T>>;
template <typename T> using ReplyTask = PassiveTask<ReplyNode<T>>;

// --

template <typename T> class ActiveTask : public Task<T> {
protected:
  moodycamel::ConcurrentQueue<std::string> _que;

public:
  using TagType = typename T::TagType;

  ActiveTask(const std::string &name, std::unique_ptr<T> &&n)
      : Task<T>(name, std::forward<decltype(n)>(n)) {}

  int operator()(int ms) {
    while (Task<T>::_node->isRunning()) {
      std::string s;
      while (_que.try_dequeue(s)) {
        Task<T>::_node->send(s.data(), s.size());
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
};

// --

template <typename T> using PushTask = ActiveTask<PushNode<T>>;
template <typename T> using PublishTask = ActiveTask<PublishNode<T>>;

// --

template <typename T> class MixedTask : public Task<T> {
protected:
  moodycamel::ConcurrentQueue<std::string> _que;

public:
  using TagType = typename T::TagType;

  MixedTask(const std::string &name, std::unique_ptr<T> &&n)
      : Task<T>(name, std::forward<decltype(n)>(n)) {}

  int operator()() {
    while (Task<T>::_node->isRunning()) {
      std::string s;
      if (_que.try_dequeue(s)) {
        Task<T>::_node->send(s.data(), s.size());
      }

      Task<T>::_node->transact();
    }
    return 0;
  }

  template <typename... Args> bool post(TagType const &tag, Args &&... args) {
    std::stringstream ss;
    msgpack::pack(ss, tag);
    ((msgpack::pack(ss, std::forward<Args>(args)), ...));
    return _que.enqueue(ss.str());
  }

  template <typename... Args> int send(TagType const &tag, Args &&... args) {
    return Task<T>::_node->transmit(tag, std::forward<Args>(args)...);
    // std::stringstream ss;
    // msgpack::pack(ss, tag);
    // ((msgpack::pack(ss, std::forward<Args>(args)), ...));
    // return Task<T>::_node->send(ss.str().data(), ss.str().size());
  }
};

// --

template <typename T> using PairTask = MixedTask<PairNode<T>>;
template <typename T> using BusTask = MixedTask<BusNode<T>>;
template <typename T> using ResponseTask = MixedTask<ResponseNode<T>>;

// --

} // namespace rpcpp
