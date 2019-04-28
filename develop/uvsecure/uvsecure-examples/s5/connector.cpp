#include <iostream>
#include <cstdlib>

#include <uvplus.hpp>

#include "cryptography.h"
#include "securelayer.h"
#include "s5acceptor.h"
#include "connector.h"

// --

struct Connector::Impl {
  uvp::TcpObject _socket;
  uvp::TimerObject _timer;
  SecureRecord _sr;
  sockaddr _dest;
  std::string _name, _peer;
  Connector *_connector = nullptr;
  std::unique_ptr<S5Acceptor> _s5Acceptor;

  Impl(uvp::Loop *loop, const struct sockaddr *dest, Connector *conn,
       bool secure)
      : _sr(secure), _socket(loop), _timer(loop), _dest(*dest),
        _connector(conn) {
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

  void collect(const char *p, size_t len) {
    auto lv = _sr.feed(p, len);
    for (auto v : lv) {
      std::string s = secure::hex_encode(v);
      std::cout << "recv from: " << peer() << std::endl << s << std::endl;

      auto s5r = (S5Record *)v.data();
      std::string from(s5r->from()->data(), s5r->from()->len());
      switch (s5r->type) {
      case S5Record::Type::Reply:
        _s5Acceptor->reply(from, s5r->data()->data(), s5r->data()->len());
        break;

      case S5Record::Type::S5ConnectorClosed:
        _s5Acceptor->shutdown(from);
        break;

      default:
        UVP_ASSERT(false);
        break;
      }
    }
  }

  void onRead(uvp::Stream *stream, ssize_t nread, const uvp::uv::BufT *buf) {
    if (nread < 0) {
      UVP_LOG_ERROR(nread);
      _socket.readStop();
      _socket.close();
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
      if (_timer.isActive()) {
        _timer.repeat(5000);
      } else {
        _timer.start(5000, 5000);
      }
      return;
    }

    if (_timer.isActive()) {
      _timer.repeat(50);
    } else {
      _timer.start(500, 50);
    }

    _socket.readStart();

    // SecureRecord复位，向对端发送hello更新密码。
    sayHello();

    // 创建s5acceptor，接受来自socks5 client端的连接
    if (!_s5Acceptor) {
      sockaddr_in addr;
      uvp::ip4Addr("0", 7001, &addr);
      _s5Acceptor =
          std::make_unique<S5Acceptor>(_socket.loop(), (const sockaddr *)&addr);
      _s5Acceptor->secureConnector(_connector);
    }
  }

  void onShutdown(uvp::Stream *stream, int status) {
    if (status < 0) {
      UVP_LOG_ERROR(status);

      // shutdown正常，那么就在onRead中close handle，
      // 这个可能会等待很长时间（keepalive-timeout）。
      // 如果在onShutdown中close handle，可能会导致对端发来的数据未被处理，
      // 但是能够及时close handle。
      // 权衡之后，等待在onRead中close handle的做法会让数据更可靠些。
      if (!_socket.isClosing()) {
        _socket.close();
      }
    }
    LOG_INFO << "client socket shutdown.";
  }

  void onClose(uvp::Handle *handle) { LOG_INFO << "handle of socket closed."; }

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

  void write(const uint8_t *p, size_t len) {
    if (std::time(nullptr) % 73 == 0) {
      auto l = _sr.update();
      write(l);
    }
    auto l = _sr.pack(p, len);
    write(l);
  }

  void shutdown() {
    int r = _socket.shutdown();
    if (r && !_socket.isClosing()) {
      UVP_LOG_ERROR(r);
      _socket.close();
    }
  }

  void onTimer(uvp::Timer *handle) {
    if (_socket.isClosing()) {
      int r = _socket.reinit();
      UVP_LOG_ERROR(r);
      r = _socket.connect(&_dest);
      UVP_LOG_ERROR(r);
    } else {
      static size_t sn = 0;
      if (sn++ < 0) {
        auto j = 0;
        u8vector v(112 + (sn % 383));
        for (auto &c : v) {
          c = j++;
        }
        write(v.data(), v.size());
      } else {
        // _timer.stop();
        // _timer.close();
        // _s5Acceptor->shutdown();
        // shutdown();
      }
    }
  }

  size_t length() const { return _sr.length() - 256 - 2 - 64; }
};

Connector::Connector(uvp::Loop *loop, const struct sockaddr *dest, bool secure)
    : _impl(std::make_unique<Connector::Impl>(loop, dest, this, secure)) {}

Connector::~Connector() {}

void Connector::secureWrite(S5Record::Type t, const std::string &from,
                            const uint8_t *p, size_t len) {
  UVP_ASSERT(from.size() < 0xFF);

  while (len > 0) {
    auto writeLen = std::min(_impl->length(), len);
    UVP_ASSERT(writeLen < 18 * 1024);

    u8vector v(S5Record::HeadLen() + from.size() + writeLen);
    auto s5r = (S5Record *)v.data();

    s5r->type = t;
    s5r->from()->len(from.size());
    std::memcpy(s5r->from()->data(), from.data(), from.size());
    s5r->data()->len(writeLen);
    std::memcpy(s5r->data()->data(), p, writeLen);
    auto l1 = s5r->size();
    auto l2 = v.size();

    UVP_ASSERT(s5r->size() == v.size());

    // v.resize(s5r->size());
    _impl->write(v.data(), v.size());

    len -= writeLen;
    p += writeLen;
  }
}
