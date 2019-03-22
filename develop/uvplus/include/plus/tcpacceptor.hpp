#pragma once

#include <string>
#include <list>
#include <vector>
#include <unordered_map>

#include <uvp.hpp>

#include "gangway.hpp"

// --

namespace uvplus {

class TcpAcceptor {
  class TcpPeer {
    uvp::TcpObject _socket;
    std::string _peer;
    TcpAcceptor &_acceptor;

    std::unique_ptr<PackInterface> _packInterface;

    void makeup(const char *p, size_t len) {
      int remain = len;
      do {
        int writed = _packInterface->feed(p, remain);
        remain -= writed;
        p += writed;

        while (true) {
          // 解析出每个包
          uvp::uv::BufT b = {0};
          if (!_packInterface->unpack(b)) {
            break;
          }

          // 打包到packet，并上传给业务处理。
          Packet packet(_peer, b);
          _acceptor.upwardEnqueue(std::move(packet));
        }
      } while (remain > 0);
    }

    void onRead(uvp::Stream *stream, ssize_t nread, const uvp::uv::BufT *buf) {
      if (nread < 0) {
        _socket.close();
        UVP_LOG_ERROR(nread);
        return;
      }

      // 整理包，并进一步处理包
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
    TcpPeer(uvp::Loop *loop, TcpAcceptor &server)
        : _socket(loop), _acceptor(server) {
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

    void packInterface(std::unique_ptr<PackInterface> &&pi) {
      _packInterface = std::forward<decltype(pi)>(pi);
    }
  };

  uvp::TcpObject _socket;
  uvp::AsyncObject _async;
  uvp::TimerObject _timer;

  uvp::Loop *_loop;
  std::unordered_map<std::string, std::unique_ptr<TcpPeer>> _clients;

  std::unique_ptr<PackInterface> _packInterface;
  Gangway _gangway;

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

    auto client = std::make_unique<TcpPeer>(_loop, *this);
    client->packInterface(_packInterface->clone());
    int r = _socket.accept(client->socket());
    if (r < 0) {
      UVP_LOG_ERROR(r);
      return;
    }
    client->socket()->readStart();

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
    std::vector<Packet> packets;
    size_t count = 0;
    while ((count = downwardDequeue(packets)) > 0) {
      for (int i = 0; i < count; ++i) {
        uvp::uv::BufT b = packets[i].releaseBuf();
        auto it = _clients.find(packets.at(i)._peer);
        if (it == _clients.end()) {
          uvp::freeBuf(b);
        } else {
          int r = it->second->socket()->write(&b, 1);
          if (r) {
            uvp::freeBuf(b);
            UVP_LOG_ERROR(r);
            break;
          }
        }
      }
    }
    notifyHandler();
  }

  void onTimer(uvp::Timer *handle) {
    if (_gangway._upward.size_approx() > 1000) {
      for (auto &c : _clients) {
        int r = c.second->socket()->readStop();
        UVP_LOG_ERROR(r);
      }
      readStopped = true;
      LOG_INFO << "clients read stoped.";
    }
    if (readStopped && _gangway._upward.size_approx() < 10) {
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

  bool upwardEnqueue(Packet &&packet) {
    while (!_gangway._upward.enqueue(std::forward<Packet>(packet))) {
      LOG_CRIT << "enqueue faild in acceptor upward queue.";
    }

    return true;
  }

  bool downwardDequeue(Packet &packet) {
    return _gangway._downward.try_dequeue(packet);
  }

  size_t downwardDequeue(std::vector<Packet> &packets) {
    size_t n = _gangway._downward.size_approx();
    if (!n) {
      return 0;
    }
    packets.resize(n);
    return _gangway._downward.try_dequeue_bulk(packets.begin(), packets.size());
  }

 public:
  enum class NotifyTag { NT_NOTHING = 0, NT_CLOSE, NT_CLIENTS_SHUTDOWN };

  TcpAcceptor(uvp::Loop *loop, const struct sockaddr *addr)
      : _socket(loop),
        _async(loop,
               std::bind(&TcpAcceptor::onAsync, this, std::placeholders::_1)),
        _timer(loop),
        _loop(loop) {
    const int backlog = 128;

    _socket.connectionCallback(std::bind(&TcpAcceptor::onConnection, this,
                                         std::placeholders::_1,
                                         std::placeholders::_2));
    _socket.shutdownCallback(std::bind(&TcpAcceptor::onShutdown, this,
                                       std::placeholders::_1,
                                       std::placeholders::_2));
    _socket.closeCallback(
        std::bind(&TcpAcceptor::onClose, this, std::placeholders::_1));

    int r = _socket.bind(addr, 0);
    UVP_LOG_ERROR_EXIT(r);
    r = _socket.listen(backlog);
    UVP_LOG_ERROR_EXIT(r);

    LOG_INFO << "listen at: " << name();

    _timer.startCallback(
        std::bind(&TcpAcceptor::onTimer, this, std::placeholders::_1));
    _timer.start(1000, 100);
  }

  std::string name() const {
    if (_name.empty()) {
      _name = _socket.name();
    }
    return _name;
  }

  void packInterface(std::unique_ptr<PackInterface> &&pi) {
    _packInterface = std::forward<decltype(pi)>(pi);
  }

  bool upwardDequeue(Packet &packet) {
    return _gangway._upward.wait_dequeue_timed(packet,
                                               std::chrono::milliseconds(500));
  }

  size_t upwardDequeue(std::vector<Packet> &packets) {
    size_t n = _gangway._upward.size_approx();
    packets.resize(std::max((size_t)1, n));
    return _gangway._upward.wait_dequeue_bulk_timed(
        packets.begin(), packets.size(), std::chrono::milliseconds(500));
  }

  int downwardEnqueue(const char *name, const char *p, size_t len) {
    uvp::uv::BufT b = {0};
    if (!_packInterface->pack(b, p, len)) {
      return -1;
    }
    Packet packet(name, b);
    while (!_gangway._downward.enqueue(std::move(packet))) {
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

}  // namespace uvplus
