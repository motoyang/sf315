#pragma once

#include <mutex>
#include <condition_variable>

#include <msgpack.hpp>

#include <uvp.hpp>

#include "gangway.hpp"

// --

namespace uvplus {

template<typename C> class TcpConnector {
  uvp::TcpObject _socket;
  uvp::AsyncObject _async;
  uvp::TimerObject _timer;

  C _codec;
  GangwayInConnector _gangway;
  std::atomic<int> _notifyTag{0};

  sockaddr _dest;
  std::string _name;
  std::string _peer;

  std::mutex _mutex;
  std::condition_variable _cond_var;
  std::unordered_map<int, uvp::uv::BufT> _reqMap;

  void dispatch(uvp::uv::BufT buf) {
    size_t offset = 0;
    int buf_type = 0;
    msgpack::object_handle oh = msgpack::unpack(buf.base, buf.len, offset);
    oh.get().convert(buf_type);
    if (buf_type == (int)uvp::BufType::BUF_REP_TYPE) {
      msgpack::object_handle oh2 = msgpack::unpack(buf.base, buf.len, offset);
      int token = 0;
      oh2.get().convert(token);

      std::lock_guard<std::mutex> lk(_mutex);
      if (_reqMap.find(token) != _reqMap.end()) {
        _reqMap[token] = buf;
        _cond_var.notify_all();
      } else {
        uvp::freeBuf(buf);
      }
    } else {
      // 进队列失败时，重新进队列直到进入队列成功。
      while (!_gangway._upward.try_enqueue(buf))
        ;
    }
  }

  void makeup(const char *p, size_t len) {
    int remain = len;
    do {
      int writed = _codec.ringBuffer().write(p, remain);
      remain -= writed;
      p += writed;

      while (true) {
        // 解析出每个包
        uvp::uv::BufT b = {0};
        if (!_codec.decode(b)) {
          break;
        }
        dispatch(b);
      }
    } while (remain > 0);
  }

  void onRead(uvp::Stream *stream, ssize_t nread, const uvp::uv::BufT *buf) {
    if (nread < 0) {
      UVP_LOG_ERROR(nread);
      _socket.readStop();
      _socket.close();
      return;
    }

    if (nread) {
      makeup(buf->base, nread);
    }
  }

  void onWrite(uvp::Stream *stream, int status, uvp::uv::BufT bufs[],
               int nbufs) {
    if (status < 0) {
      UVP_LOG_ERROR(status);
    }

    for (int i = 0; i < nbufs; ++i) {
      uvp::freeBuf(bufs[i]);
    }
  }

  void onConnect(uvp::Stream *stream, int status) {
    if (status < 0) {
      UVP_LOG_ERROR(status);
      return;
    }
    LOG_INFO << "client socket connected to: " << peer();
    // nameOfPeer(AF_INET, &_socket);
    // _name = nameOfSock(AF_INET, &_socket);
    _socket.readStart();

    _timer.start(1000, 5000);
  }

  void onShutdown(uvp::Stream *stream, int status) {
    if (status < 0) {
      UVP_LOG_ERROR(status);
    }
    LOG_INFO << "client socket shutdown.";
    _socket.close();
  }

  void onClose(uvp::Handle *handle) { LOG_INFO << "handle of socket closed."; }

  void onTimer(uvp::Timer *handle) {
    if (!_socket.isWritable() || !_socket.isReadable()) {
      int r = _socket.reinit();
      UVP_LOG_ERROR(r);
      r = _socket.connect(&_dest);
      UVP_LOG_ERROR(r);
      return;
    }
  }

  void onAsync(uvp::Async *handle) {
    if (_notifyTag == 1) {
      _timer.stop();
      _timer.close();

      _async.close();

      _socket.readStop();
      int r = _socket.shutdown();
      if (r) {
        UVP_LOG_ERROR(r);
        _socket.close();
      }

      return;
    }

    if (!_socket.isWritable()) {
      return;
    }

    uvp::uv::BufT bufs[10] = {0};
    size_t count = 0;
    while ((count = _gangway._downward.try_dequeue_bulk(bufs, COUNT_OF(bufs))) >
           0) {
      int r = _socket.write(bufs, count);
      if (r) {
        for (int i = 0; i < count; ++i) {
          uvp::freeBuf(bufs[i]);
        }
        UVP_LOG_ERROR(r);
      }
    }
  }

public:
  TcpConnector(uvp::Loop *loop, const struct sockaddr *dest)
      : _socket(loop), _async(loop, std::bind(&TcpConnector::onAsync, this,
                                              std::placeholders::_1)),
        _timer(loop), _dest(*dest) {
    _socket.connectCallback(std::bind(&TcpConnector::onConnect, this,
                                      std::placeholders::_1,
                                      std::placeholders::_2));
    _socket.shutdownCallback(std::bind(&TcpConnector::onShutdown, this,
                                       std::placeholders::_1,
                                       std::placeholders::_2));
    _socket.closeCallback(
        std::bind(&TcpConnector::onClose, this, std::placeholders::_1));
    _socket.readCallback(std::bind(&TcpConnector::onRead, this,
                                   std::placeholders::_1, std::placeholders::_2,
                                   std::placeholders::_3));
    _socket.writeCallback(std::bind(
        &TcpConnector::onWrite, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

    _timer.startCallback(
        std::bind(&TcpConnector::onTimer, this, std::placeholders::_1));

    int r = _socket.connect(dest);
    UVP_LOG_ERROR_EXIT(r);
  }

  std::string name() {
    if (_name.empty()) {
      _name = _socket.name();
    }
    return _name;
  }

  std::string peer() {
    if (_peer.empty()) {
      _peer = _socket.peer();
    }
    return _peer;
  }

  void notify(int tag) {
    _notifyTag = tag;
    _async.send();
  }

  size_t upwardDequeue(uvp::uv::BufT bufs[], size_t nbufs) {
    return _gangway._upward.wait_dequeue_bulk_timed(
        bufs, nbufs, std::chrono::milliseconds(500));
  }

  int downwardEnqueue(const char *p, size_t len) {
    size_t sleep_ms = _gangway._downward.size_approx() / 10;
    if (sleep_ms) {
      std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
    }

    uvp::uv::BufT b = {0};
    if (!_codec.encode(b, p, len)) {
      return 0;
    }
    // 进队列失败时，重新进队列直到进入队列成功。
    while (!_gangway._downward.enqueue(b)) {
      LOG_CRIT << "enqueue faild in connector downward queue.";
    }

    int r = _async.send();
    UVP_LOG_ERROR(r);
    return r;
  }

  template <typename T, typename... Args>
  int transmit(uvp::BufType bt, T const &tag, Args &&... args) {
    static std::atomic<int> token_seed = 0;
    int token = ++token_seed;

    std::stringstream ss;
    msgpack::pack(ss, (int)bt);
    msgpack::pack(ss, token);
    msgpack::pack(ss, tag);
    ((msgpack::pack(ss, std::forward<Args>(args)), ...));

    int r = downwardEnqueue(ss.str().data(), ss.str().length());
    UVP_LOG_ERROR(r);
    return r;
  }

  template <typename R, typename T, typename... Args>
  int request(R &result, T const &tag, Args &&... args) {
    static std::atomic<int> token_seed = 0;
    int token = ++token_seed;

    uvp::uv::BufT buf = {0};
    do {
      std::lock_guard<std::mutex> lk(_mutex);
      _reqMap.insert({token, buf});
    } while (false);

    std::stringstream ss;
    msgpack::pack(ss, (int)uvp::BufType::BUF_REQ_TYPE);
    msgpack::pack(ss, token);
    msgpack::pack(ss, tag);
    ((msgpack::pack(ss, std::forward<Args>(args)), ...));
    int r = downwardEnqueue(ss.str().data(), ss.str().length());
    if (r) {
      UVP_LOG_ERROR(r);
      return r;
    }

    uvp::uv::BufT b = {0};
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
    assert(buf_type == (int)uvp::BufType::BUF_REP_TYPE);
    msgpack::object_handle oh2 = msgpack::unpack(b.base, b.len, offset);
    oh2.get().convert(token_return);
    assert(token == token_return);
    std::string s;
    msgpack::object_handle oh3 = msgpack::unpack(b.base, b.len, offset);
    oh3.get().convert(s);
    uvp::freeBuf(b);

    msgpack::object_handle oh4 = msgpack::unpack(s.data(), s.length());
    oh4.get().convert(result);
    return 0;
  }
};

// --

} // namespace uvplus
