#pragma once

// #include <ringbuffer.hpp>
#include <uv.hpp>
#include <gangway.hpp>

#include <msgpack.hpp>
// --

// --

class TcpConnector {
  TcpT _socket;
  AsyncT _async;
  TimerT _timer;

  CodecI &_codec;
  GangwayInConnector _gangway;
  std::atomic<int> _notifyTag{0};

  sockaddr _dest;
  std::string _name;
  std::string _peer;

  std::mutex _mutex;
  std::condition_variable _cond_var;
  std::unordered_map<int, BufT> _reqMap;

  void dispatch(BufT buf);
  void makeup(const char *p, size_t len);

  void onRead(ssize_t nread, const BufT *buf);
  void onWrite(int status, BufT bufs[], int nbufs);
  void onConnect(int status);
  void onShutdown(int status);
  void onClose();
  void onTimer();
  void onAsync();

public:
  TcpConnector(LoopI *loop, const struct sockaddr *addr, CodecI &codec);
  std::string name();
  std::string peer();
  void notify(int tag);

  size_t upwardDequeue(BufT bufs[], size_t nbufs);
  int downwardEnqueue(const char *p, size_t len);

  template <typename T, typename... Args>
  int transmit(BufType bt, T const &tag, Args &&... args) {
    static std::atomic<int> token_seed = 0;
    int token = ++token_seed;

    std::stringstream ss;
    msgpack::pack(ss, (int)bt);
    msgpack::pack(ss, token);
    msgpack::pack(ss, tag);
    ((msgpack::pack(ss, std::forward<Args>(args)), ...));

    int r = downwardEnqueue(ss.str().data(), ss.str().length());
    LOG_IF_ERROR(r);
    return r;
  }

  template <typename R, typename T, typename... Args>
  int request(R &result, T const &tag, Args &&... args) {
    static std::atomic<int> token_seed = 0;
    int token = ++token_seed;

    BufT buf = {0};
    do {
      std::lock_guard<std::mutex> lk(_mutex);
      _reqMap.insert({token, buf});
    } while (false);

    std::stringstream ss;
    msgpack::pack(ss, (int)BufType::BUF_REQ_TYPE);
    msgpack::pack(ss, token);
    msgpack::pack(ss, tag);
    ((msgpack::pack(ss, std::forward<Args>(args)), ...));
    int r = downwardEnqueue(ss.str().data(), ss.str().length());
    if (r) {
      LOG_IF_ERROR(r);
      return r;
    }

    BufT b = {0};
    bool waited = false;
    do {
      std::unique_lock<std::mutex> lk(_mutex);
      waited = _cond_var.wait_for(lk, std::chrono::milliseconds(5000),
                                  [&b, token, this]() {
                                    b = _reqMap.at(token);
                                    return b.len;
                                  });
      _reqMap.erase(token);
    } while (false);

    if (!waited) {
      return -1;
    }

    size_t offset = 0;
    int buf_type = 0;
    int token_return = 0;
    msgpack::object_handle oh = msgpack::unpack(b.base, b.len, offset);
    oh.get().convert(buf_type);
    assert(buf_type == (int)BufType::BUF_REP_TYPE);
    msgpack::object_handle oh2 = msgpack::unpack(b.base, b.len, offset);
    oh2.get().convert(token_return);
    assert(token == token_return);
    std::string s;
    msgpack::object_handle oh3 = msgpack::unpack(b.base, b.len, offset);
    oh3.get().convert(s);
    freeBuf(b);

    msgpack::object_handle oh4 = msgpack::unpack(s.data(), s.length());
    oh4.get().convert(result);
    return 0;
  }
};

// --

int tcp_client();
