#pragma once

bool initRep(int i);

// --

namespace rpcpp2 {

// --

class Task {
public:
  virtual void close() = 0;
};

// --

class PushTask: public Task {
  PushNode _node;
  std::function<std::string()> _push;
  std::function<void()> _interval;

public:
  PushTask(std::string const& url);
  int operator() ();

  template<typename F, typename... Args>
  void push(F&& f, Args&&... args) {
    _push = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
  }

  template<typename F, typename... Args>
  void interval(F&& f, Args&&... args) {
    _interval = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
  }

  virtual void close() override;
};

// --

class PullTask: public Task{
  PullNode _node;
  std::function<bool()> _init;
  std::function<void(const char*, size_t len)> _pull;

public:
  PullTask(const std::string& url);
  int operator() ();

  template<typename F, typename... Args>
  void init(F&& f, Args&&... args) {
    _init = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
  }

  void pull(std::function<void(const char*, size_t len)>&& f) {
    _pull = f;
  }

  virtual void close() override;
};

// --

template<typename Node>
class PairTask: public Task {
  Node _node;
  std::function<void()> _interval;
  std::function<std::string()> _say;
  std::function<void(const char*, size_t len)> _hear;

public:
  PairTask(std::string const& url) : _node(url) {}
  int operator() () {
    LOG_TRACK;
    while (_node.run()) {
      char *buf = NULL;
      size_t sz = 0;
      int rv = 0;
      if ((rv = _node.recv(&buf, &sz)) == 0) {
        _hear(buf, sz);
      }
      if (rv == NNG_ECLOSED) {
        break;
      }

      _interval();

      std::string s = _say();
      if ((rv = _node.send(s.data(), s.size())) != 0) {
        if (rv == NNG_ECLOSED) {
          break;
        }
      }

    }
    _node.close();

    LOG_INFO << "operator() will return with 0.";
    return 0;
  }

  void hear(std::function<void(const char*, size_t len)>&& f) {
    _hear = f;
  }

  template<typename F, typename... Args>
  void say(F&& f, Args&&... args) {
    _say = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
  }

  template<typename F, typename... Args>
  void interval(F&& f, Args&&... args) {
    _interval = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
  }

  virtual void close() override {
    _node.close();
  }
};

// --

class TaskManager {
  std::map<std::string, std::unique_ptr<Task>> _tasks;

public:
  void AddTask(const std::string& n, std::unique_ptr<Task>&& t) {
    _tasks.insert(std::make_pair(n, std::forward<decltype(t)>(t)));
  };

  void stop() {
    for (auto& p: _tasks) {
      p.second->close();
    }
  }
};

// --

}
