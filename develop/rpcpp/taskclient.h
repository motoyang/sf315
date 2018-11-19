#pragma once

// --
namespace rpcpp2 {
namespace client {

// --

class SubscribeTask: public Task {
  SubscribeNode _node;
  std::function<bool()> _init;
  std::function<void(const char*, size_t len)> _subscribe;

public:
  SubscribeTask(const std::string& url);
  int operator() ();

  template<typename F, typename... Args>
  void init(F&& f, Args&&... args) {
    _init = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
  }

  void subscribe(std::function<void(const char*, size_t len)>&& f) {
    _subscribe = f;
  }

  virtual void close() override;
};

// --

class RequestTask: public Task {
  RequestNode _node;
  std::function<int()> _request;

  msgpack::object_handle sendAndRecv(const std::string& s);

public:
  RequestTask(const std::string& url);
  int operator() ();
  virtual void close() override;

  template<typename F, typename... Args>
  void request(F&& f, Args&&... args) {
    _request = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
  }

  template <typename R, typename... Args>
  int call(const std::string &fn, R &result, Args&&... args) {
    std::stringstream ss;
    msgpack::pack(ss, fn);
    ((msgpack::pack(ss, std::forward<Args>(args)), ...));

    msgpack::object_handle oh = sendAndRecv(ss.str());
    oh.get().convert(result);

    return 0;
  }

  template <typename... Args>
  int callAndClose(const std::string &fn, Args&&... args) {
    std::stringstream ss;
    msgpack::pack(ss, fn);
    ((msgpack::pack(ss, std::forward<Args>(args)), ...));

    std::string s = ss.str();
    int rv = 0;
    if ((rv = _node.send(s.data(), s.size())) == NNG_ECLOSED) {
      LOG_WARN << _node.strerror(rv);
    }

    return 0;
  }
};

// --

}
}
