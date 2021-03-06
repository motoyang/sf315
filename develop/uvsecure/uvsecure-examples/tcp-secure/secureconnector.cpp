#include "uvplus.hpp"
#include "cryptography.h"
#include "securelayer.h"
#include "secureconnector.h"

// --

struct SecureConnector::Impl {
private:
  uvp::TcpObject _socket;
  uvp::AsyncObject _async;
  uvp::TimerObject _timer;

  SecureRecord _sr;
  size_t _recordCount = 0;

  moodycamel::BlockingConcurrentQueue<u8vector> _upward;
  moodycamel::ConcurrentQueue<u8vector> _downward;
  std::atomic<NotifyTag> _notifyTag{SecureConnector::NotifyTag::NOTHING};

  sockaddr _dest;
  std::string _name;
  std::string _peer;
  uvplus::TcpStatusInterface *_tsi = nullptr;

  void collect(const char *p, size_t len) {
    auto lv = _sr.feed(p, len);
    for (auto v : lv) {
      // 进队列失败时，重新进队列直到进入队列成功。
      while (!_upward.try_enqueue(std::move(v)))
        ;
    }
  }

  void onRead(uvp::Stream *stream, ssize_t nread, const uvp::uv::BufT *buf) {
    if (nread < 0) {
      UVP_LOG_ERROR(nread);
      _socket.readStop();
      _socket.close();
      _timer.start(5000, 5000);

      if (_tsi) {
        _tsi->disconnected(peer());
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
    if (_tsi) {
      _tsi->connected(peer());
    }

    _socket.readStart();
    _async.send();
    _timer.stop();

    // SecureRecord复位，向对端发送hello更新密码。
    sayHello();
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
    if (_notifyTag == NotifyTag::CLOSE) {
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

    size_t n = _downward.size_approx();
    u8vlist bufs(std::max((size_t)1, n));
    size_t count = 0;
    while ((count = _downward.try_dequeue_bulk(bufs.begin(), bufs.size())) >
           0) {
      bufs.resize(count);
      for (const auto &v : bufs) {
        if (++_recordCount % 37 == 0) {
          auto l = _sr.update();
          write(l);
        }
        auto l = _sr.pack(v.data(), v.size());
        write(l);
      }
    }
  }

  void sayHello() {
    auto l = _sr.reset();
    write(l);
  }

  void write(const std::list<uvp::uv::BufT> &l) {
    for (auto b : l) {
      int r = _socket.write(&b, 1);
      if (r) {
        uvp::freeBuf(b);
        UVP_LOG_ERROR(r);
      }
    }
  }

public:
  Impl(uvp::Loop *loop, const struct sockaddr *dest, bool secure)
      : _sr(secure), _socket(loop),
        _async(loop, std::bind(&Impl::onAsync, this, std::placeholders::_1)),
        _timer(loop), _dest(*dest) {
    _socket.connectCallback(std::bind(
        &Impl::onConnect, this, std::placeholders::_1, std::placeholders::_2));
    _socket.shutdownCallback(std::bind(
        &Impl::onShutdown, this, std::placeholders::_1, std::placeholders::_2));
    _socket.closeCallback(
        std::bind(&Impl::onClose, this, std::placeholders::_1));
    _socket.readCallback(std::bind(&Impl::onRead, this, std::placeholders::_1,
                                   std::placeholders::_2,
                                   std::placeholders::_3));
    _socket.writeCallback(std::bind(
        &Impl::onWrite, this, std::placeholders::_1, std::placeholders::_2,
        std::placeholders::_3, std::placeholders::_4));

    _timer.startCallback(
        std::bind(&Impl::onTimer, this, std::placeholders::_1));

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

  void notify(NotifyTag tag) {
    _notifyTag = tag;
    _async.send();
  }

  void tcpStatusInterface(uvplus::TcpStatusInterface *tsi) { _tsi = tsi; }

  size_t read(u8vlist &l) {
    size_t n = _upward.size_approx();
    l.resize(std::max((size_t)1, n));
    n = _upward.wait_dequeue_bulk_timed(l.begin(), l.size(),
                                        std::chrono::milliseconds(500));
    l.resize(n);
    return n;
  }

  int write(const uint8_t *p, size_t len) {
    while (!_downward.enqueue(u8vector(p, p + len))) {
      LOG_CRIT << "enqueue faild in connector downward queue.";
    }

    int r = _async.send();
    UVP_LOG_ERROR(r);
    return r;
  }
};

// --

SecureConnector::SecureConnector(uvp::Loop *loop, const struct sockaddr *dest,
                                 bool secure)
    : _impl(std::make_unique<SecureConnector::Impl>(loop, dest, secure)) {}

SecureConnector::~SecureConnector() {}

std::string SecureConnector::name() { return _impl->name(); }

std::string SecureConnector::peer() { return _impl->peer(); }

void SecureConnector::notify(NotifyTag tag) { _impl->notify(tag); }

void SecureConnector::tcpStatusInterface(uvplus::TcpStatusInterface *tni) {
  _impl->tcpStatusInterface(tni);
}

size_t SecureConnector::read(u8vlist &l) { return _impl->read(l); }

int SecureConnector::write(const uint8_t *p, size_t len) {
  return _impl->write(p, len);
}