#pragma once

#include <uvp.hpp>

#include "chunk.hpp"

// --

namespace uvplus {

class TcpConnector {
  uvp::TcpObject _socket;
  uvp::AsyncObject _async;
  uvp::TimerObject _timer;

  // Chunk _chunk;
  // Record _record;
  RecordLayer _r2;

  moodycamel::BlockingConcurrentQueue<u8vector> _upward;
  moodycamel::ConcurrentQueue<uvp::uv::BufT> _downward;
  std::atomic<int> _notifyTag{0};

  sockaddr _dest;
  std::string _name;
  std::string _peer;
  TcpNotifyInterface* _tni = nullptr;

  void collect(const char *p, size_t len) {
    auto lv = _r2.feed(p, len);
    for (auto v : lv) {
      // 进队列失败时，重新进队列直到进入队列成功。
      while (!_upward.try_enqueue(std::move(v)))
        ;
    }
  }
  /*
    void collect(const char *p, size_t len) {
      auto lv = _chunk.feed(p, len);
      for (const auto &v : lv) {
        auto lb = _record.feed(v.data(), v.size());
        for (auto b : lb) {
          // 进队列失败时，重新进队列直到进入队列成功。
          while (!_upward.try_enqueue(std::move(b)))
            ;
        }
      }
    }
  */
  void onRead(uvp::Stream *stream, ssize_t nread, const uvp::uv::BufT *buf) {
    if (nread < 0) {
      UVP_LOG_ERROR(nread);
      _socket.readStop();
      _socket.close();
      _timer.start(5000, 5000);

      if (_tni) {
        _tni->disconnected(peer());
      }

      return;
    }

    if (nread) {
      collect(buf->base, nread);
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
      _socket.close();
      _timer.start(5000, 5000);
      return;
    }

    LOG_INFO << "client socket connected to: " << peer();
    if(_tni) {
      _tni->connected(peer());
    }

    _socket.readStart();
    _async.send();
    _timer.stop();
    _r2.reset();
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
      if (r && !_socket.isClosing()) {
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
    while ((count = _downward.try_dequeue_bulk(bufs, COUNT_OF(bufs))) > 0) {
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
      : //_chunk(256), _record(2048, 256),
       _socket(loop),
        _async(loop,
               std::bind(&TcpConnector::onAsync, this, std::placeholders::_1)),
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

  void tcpNotifyInterface(TcpNotifyInterface* tni) {
    _tni = tni;
  }

  template<typename It> size_t read(It first, size_t max) {
    return _upward.wait_dequeue_bulk_timed(first, max,
                                           std::chrono::milliseconds(500));
  }
/*
  size_t read(u8vector bufs[], size_t nbufs) {
    return _upward.wait_dequeue_bulk_timed(bufs, nbufs,
                                           std::chrono::milliseconds(500));
  }

  int write(const uint8_t *p, size_t len) {
    auto lv = _record.slice(p, len);
    for (const auto &v : lv) {
      auto b = _chunk.pack(v.data(), v.size());

      // 进队列失败时，重新进队列直到进入队列成功。
      while (!_downward.enqueue(b)) {
        LOG_CRIT << "enqueue faild in connector downward queue.";
      }
    }

    int r = _async.send();
    UVP_LOG_ERROR(r);
    return r;
  }
*/
  int write(const uint8_t *p, size_t len) {
    auto lb = _r2.slice(p, len);
    for (auto b : lb) {
      // 进队列失败时，重新进队列直到进入队列成功。
      while (!_downward.enqueue(b)) {
        LOG_CRIT << "enqueue faild in connector downward queue.";
      }
    }

    int r = _async.send();
    UVP_LOG_ERROR(r);
    return r;
  }
};

// --

} // namespace uvplus
