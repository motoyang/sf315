#pragma once

// --

namespace rpcpp2 {
namespace server {

// --

using PairServerTask = PairTask<PairNode>;


// --

class PublishTask: public Task {
  PublishNode _node;
  std::function<void()> _interval;
  std::function<std::string()> _publish;

public:
  PublishTask(std::string const& url);
  int operator() ();

  template<typename F, typename... Args>
  void interval(F&& f, Args&&... args) {
    _interval = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
  }

  template<typename F, typename... Args>
  void publish(F&& f, Args&&... args) {
    _publish = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
  }

  virtual void close() override;
};

// --

class ReplyTask: public Task {
  ReplyNode _node;
  std::function<bool()> _init;
  std::function<std::string(const char*, size_t len)> _reply;

public:
  ReplyTask(const std::string& url);
  int operator() ();

  template<typename F, typename... Args>
  void init(F&& f, Args&&... args) {
    _init = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
  }

  void reply(std::function<std::string(const char*, size_t len)>&& f) {
    _reply = f;
  }

  virtual void close() override;
};

// --

}
}
