#include "securelayer.h"
#include "secureacceptor.h"

// --

struct SecureAcceptor::Impl {
private:
  class TcpPeer {
    uvp::TcpObject _socket;
    std::string _peer;
    Impl &_acceptor;

    SecureRecord _sr;
    size_t _recordCount = 0;

    void collect(const char *p, size_t len) {
      auto lv = _sr.feed(p, len);
      for (auto v : lv) {
        // 打包到packet，并上传给业务处理。
        _acceptor.upwardEnqueue(uvplus::Packet(peer(), std::move(v)));
      }
    }

    void onRead(uvp::Stream *stream, ssize_t nread, const uvp::uv::BufT *buf) {
      if (nread < 0) {
        _socket.close();
        UVP_LOG_ERROR(nread);
        return;
      }

      // 整理包，并进一步处理包
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

    void onShutdown(uvp::Stream *stream, int status) {
      if (status < 0) {
        UVP_LOG_ERROR(status);
      }
      LOG_INFO << "agent socket shutdown." << _peer;
      _socket.close();
    }

    void onClose(uvp::Handle *handle) {
      // 需要在onClose中先hold住clientagent，否则对象被销毁，onClose代码执行非法！
      auto p = _acceptor.removeClient(_peer);
      LOG_INFO << "handle of agent socket closed." << _peer;
    }

  public:
    TcpPeer(uvp::Loop *loop, Impl &server, bool secure)
        : _sr(secure), _socket(loop), _acceptor(server) {
      _socket.writeCallback(std::bind(
          &TcpPeer::onWrite, this, std::placeholders::_1, std::placeholders::_2,
          std::placeholders::_3, std::placeholders::_4));
      _socket.readCallback(
          std::bind(&TcpPeer::onRead, this, std::placeholders::_1,
                    std::placeholders::_2, std::placeholders::_3));
      _socket.shutdownCallback(std::bind(&TcpPeer::onShutdown, this,
                                         std::placeholders::_1,
                                         std::placeholders::_2));
      _socket.closeCallback(
          std::bind(&TcpPeer::onClose, this, std::placeholders::_1));
    }

    uvp::Tcp *socket() const { return (uvp::Tcp *)&_socket; }

    std::string peer() {
      if (_peer.empty()) {
        _peer = _socket.peer();
      }
      return _peer;
    }

    void write(const uint8_t *p, size_t len) {
      if (++_recordCount % 73 == 0) {
        auto l = _sr.update();
        write(l);
      }
      auto l = _sr.pack(p, len);
      write(l);
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
  };

  uvp::TcpObject _socket;
  uvp::AsyncObject _async;
  uvp::TimerObject _timer;

  uvp::Loop *_loop;
  std::unordered_map<std::string, std::unique_ptr<TcpPeer>> _clients;
  bool _secure;

  moodycamel::BlockingConcurrentQueue<uvplus::Packet> _upward;
  moodycamel::ConcurrentQueue<uvplus::Packet> _downward;

  bool readStopped = false;
  mutable std::string _name;
  std::atomic<int> _notifyTag{0};

  void notifyHandler() {
    if (_notifyTag == (int)NotifyTag::NT_NOTHING) {
      return;
    }

    if (_notifyTag == (int)NotifyTag::NT_CLOSE) {
      clientsShutdown();
      _socket.close();
      _timer.stop();
      _timer.close();
      _async.close();
      _notifyTag.store(int(NotifyTag::NT_NOTHING));
      return;
    }

    if (_notifyTag == (int)NotifyTag::NT_CLIENTS_SHUTDOWN) {
      clientsShutdown();
      _notifyTag.store(int(NotifyTag::NT_NOTHING));
      return;
    }
  }

  void clientsShutdown() {
    for (auto &c : _clients) {
      int r = c.second->socket()->readStop();
      UVP_LOG_ERROR(r);
      r = c.second->socket()->shutdown();
      if (r) {
        c.second->socket()->close();
        UVP_LOG_ERROR(r);
      }
    }
  }

  void onConnection(uvp::Stream *stream, int status) {
    if (status < 0) {
      UVP_LOG_ERROR(status);
      return;
    }

    auto client = std::make_unique<TcpPeer>(_loop, *this, _secure);
    int r = _socket.accept(client->socket());
    if (r < 0) {
      UVP_LOG_ERROR(r);
      return;
    }
    client->socket()->readStart();
    client->sayHello();

    LOG_INFO << "accept connection from: " << client->peer();
    addClient(std::move(client));
  }

  void onShutdown(uvp::Stream *stream, int status) {
    if (status < 0) {
      UVP_LOG_ERROR(status);
    }
    LOG_INFO << "listen socket shutdown.";
    _socket.close();
  }

  void onClose(uvp::Handle *handle) {
    LOG_INFO << "handle of listen socket closed.";
  }

  void onAsync(uvp::Async *handle) {
    std::list<uvplus::Packet> packets;
    size_t count = 0;
    while ((count = downwardDequeue(packets)) > 0) {
      for (auto &p : packets) {
        auto it = _clients.find(p._peer);
        if (it == _clients.end()) {
          // LOG_INFO << "client " << p._peer << "has closed.";
          p._buf.clear();
        } else {
          it->second->write(p._buf.data(), p._buf.size());
        }
      }
      // _async.send();
      // for (int i = 0; i < count; ++i) {
      //   auto it = _clients.find(packets.at(i)._peer);
      //   if (it == _clients.end()) {
      //     LOG_INFO << "client " << packets.at(i)._peer << "has closed.";
      //   } else {
      //     u8vector v(std::move(packets.at(i)._buf));
      //     it->second->write(v.data(), v.size());
      //   }
      // }
    }
    notifyHandler();
  }

  void onTimer(uvp::Timer *handle) {
    if (_upward.size_approx() > 1000) {
      for (auto &c : _clients) {
        int r = c.second->socket()->readStop();
        UVP_LOG_ERROR(r);
      }
      readStopped = true;
      LOG_INFO << "clients read stoped.";
    }
    if (readStopped && _upward.size_approx() < 10) {
      for (auto &c : _clients) {
        int r = c.second->socket()->readStart();
        UVP_LOG_ERROR(r);
      }
      readStopped = false;
      LOG_INFO << "clients read started.";
    }
  }

  void addClient(std::unique_ptr<TcpPeer> &&client) {
    std::string n = client->peer();

    if (auto i = _clients.insert({n, std::forward<decltype(client)>(client)});
        !i.second) {
      LOG_CRIT << "client agent has a same name: " << n;
    }
    LOG_INFO << "add client: " << n;
  }

  std::unique_ptr<TcpPeer> removeClient(const std::string &name) {
    auto i = _clients.find(name);
    if (i == _clients.end()) {
      LOG_CRIT << "can't find the client: " << name;
      return std::unique_ptr<TcpPeer>();
    }
    auto p = std::move(i->second);

    int num = _clients.erase(name);
    LOG_INFO << "remove " << num << " client: " << name;

    return p;
  }

  bool upwardEnqueue(uvplus::Packet &&packet) {
    while (!_upward.enqueue(std::forward<uvplus::Packet>(packet))) {
      LOG_CRIT << "enqueue faild in acceptor upward queue.";
    }

    return true;
  }
  /*
    bool downwardDequeue(uvplus::Packet &packet) {
      return _downward.try_dequeue(packet);
    }
  */
  size_t downwardDequeue(std::list<uvplus::Packet> &packets) {
    size_t n = _downward.size_approx();
    packets.resize(std::max((size_t)1, n));
    n = _downward.try_dequeue_bulk(packets.begin(), packets.size());
    packets.resize(n);
    return n;
  }

public:
  Impl(uvp::Loop *loop, const struct sockaddr *addr, bool secure)
      : _secure(secure), _socket(loop),
        _async(loop, std::bind(&Impl::onAsync, this, std::placeholders::_1)),
        _timer(loop), _loop(loop) {
    const int backlog = 128;

    _socket.connectionCallback(std::bind(&Impl::onConnection, this,
                                         std::placeholders::_1,
                                         std::placeholders::_2));
    _socket.shutdownCallback(std::bind(
        &Impl::onShutdown, this, std::placeholders::_1, std::placeholders::_2));
    _socket.closeCallback(
        std::bind(&Impl::onClose, this, std::placeholders::_1));

    int r = _socket.bind(addr, 0);
    UVP_LOG_ERROR_EXIT(r);
    r = _socket.listen(backlog);
    UVP_LOG_ERROR_EXIT(r);

    LOG_INFO << "listen at: " << name();

    _timer.startCallback(
        std::bind(&Impl::onTimer, this, std::placeholders::_1));
    _timer.start(1000, 100);
  }

  std::string name() const {
    if (_name.empty()) {
      _name = _socket.name();
    }
    return _name;
  }
  /*
    bool read(uvplus::Packet &packet) {
      return _upward.wait_dequeue_timed(packet, std::chrono::milliseconds(500));
    }
  */
  size_t read(std::list<uvplus::Packet> &packets) {
    size_t n = _upward.size_approx();
    packets.resize(std::max((size_t)1, n));
    n = _upward.wait_dequeue_bulk_timed(packets.begin(), packets.size(),
                                        std::chrono::milliseconds(500));
    packets.resize(n);
    return n;
  }

  int write(const char *name, const uint8_t *p, size_t len) {
    u8vector buf(p, p + len);
    uvplus::Packet packet(name, std::move(buf));
    while (!_downward.enqueue(std::move(packet))) {
      LOG_CRIT << "enqueue faild in acceptor downward queue.";
    }

    int r = _async.send();
    UVP_LOG_ERROR(r);
    return r;
  }

  int notify(int tag) {
    _notifyTag.store(tag);
    int r = _async.send();
    UVP_LOG_ERROR(r);
    return r;
  }
};

// --

SecureAcceptor::SecureAcceptor(uvp::Loop *loop, const struct sockaddr *addr,
                               bool secure)
    : _impl(std::make_unique<SecureAcceptor::Impl>(loop, addr, secure)) {}

SecureAcceptor::~SecureAcceptor() {}

std::string SecureAcceptor::name() const { return _impl->name(); }
/*
bool SecureAcceptor::read(uvplus::Packet &packet) {
  return _impl->read(packet);
}
*/
size_t SecureAcceptor::read(std::list<uvplus::Packet> &packets) {
  return _impl->read(packets);
}

int SecureAcceptor::write(const char *name, const uint8_t *p, size_t len) {
  return _impl->write(name, p, len);
}

int SecureAcceptor::notify(int tag) { return _impl->notify(tag); }
