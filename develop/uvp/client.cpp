#include <iostream>

#include <utilites.hpp>
#include <misc.hpp>

#include "client.h"

// --

// --

TcpClient::TcpClient(LoopI *loop, const struct sockaddr *dest, CodecI &codec)
    : _socket(loop), _timer(loop), _idler(loop), _ringbuffer(codec.size()),
      _codec(codec) {
  // _socket.setDefaultSize(64, 3);
  std::srand(std::time(nullptr));

  _msgList.push_back("test");
  _msgList.push_back("Hello");
  _msgList.push_back("World!");
  _msgList.push_back("test_abc");
  _msgList.push_back("Hello, everyon");
  _msgList.push_back("World! Heart!");
  _msgList.push_back("abcdefghij0123456789");

  _socket.connectCallback(
      std::bind(&TcpClient::onConnect, this, std::placeholders::_1));
  _socket.shutdownCallback(
      std::bind(&TcpClient::onShutdown, this, std::placeholders::_1));
  _socket.closeCallback(std::bind(&TcpClient::onClose, this));
  _socket.readCallback(std::bind(&TcpClient::onRead, this,
                                 std::placeholders::_1, std::placeholders::_2));
  _socket.writeCallback(std::bind(&TcpClient::onWrite, this,
                                  std::placeholders::_1, std::placeholders::_2,
                                  std::placeholders::_3));

  _timer.timerCallback(std::bind(&TcpClient::onTimer, this));
  _idler.idleCallback(std::bind(&TcpClient::onTimer, this));

  _socket.connect(dest);
}

void TcpClient::onTimer() {
  static unsigned int sn = 0;
  char sn_buf[20] = {0};
  std::sprintf(sn_buf, "%u", sn++);

  int rv1 = std::rand() % 26;
  int rv2 = std::rand() % (_codec.size() - strlen(sn_buf) - 21) + 1;
  std::string msg(rv2, '=');
  
  std::string s(sn_buf);
  s += ", " + _name + ", " + msg;
  BufT buf = _codec.encode(s.data(), s.length());
  int r = _socket.write(&buf, 1);
  if (r) {
    freeBuf(buf);
    LOG_IF_ERROR(r);
  }
}

void TcpClient::doBussiness(const char *p, size_t len) {
  int remain = len;
  do {
    int writed = _ringbuffer.write(p, remain);
    remain -= writed;
    p += writed;

    while (true) {
      // 解析出每个包
      BufT b = _codec.decode(&_ringbuffer);
      if (!b.base) {
        break;
      }

      char buf[512] = {0};
      memcpy(buf, b.base, b.len);
      std::cout << _name << " <- " << buf << std::endl;

      freeBuf(b);
    }
  } while (remain > 0);
}

void TcpClient::onConnect(int status) {
  if (status < 0) {
    LOG_IF_ERROR(status);
    return;
  }
  LOG_INFO << "client socket connected.";

  _name = nameOfSock(AF_INET, &_socket);
  _socket.readStart();

  _timer.start(1000, 1);
  _idler.start();
}

void TcpClient::onShutdown(int status) {
  if (status < 0) {
    LOG_IF_ERROR(status);
  }
  LOG_INFO << "client socket shutdown.";
  _socket.close();

  _timer.stop();
  _timer.close();

  _idler.stop();
  _idler.close();
}

void TcpClient::onClose() { LOG_INFO << "handle of socket closed."; }

void TcpClient::onRead(ssize_t nread, const BufT *buf) {
  if (nread < 0) {
    if (UV_EOF == nread || UV_ECONNRESET == nread) {
      int r = _socket.shutdown();
      if (r) {
        LOG_IF_ERROR(r);
        _socket.close();
      }
      return;
    }
    LOG_IF_ERROR(nread);
  }
  if (nread) {
    doBussiness(buf->base, nread);
  }
}

void TcpClient::onWrite(int status, BufT bufs[], int nbufs) {
  if (status < 0) {
    LOG_IF_ERROR(status);
  }

  for (int i = 0; i < nbufs; ++i) {
    freeBuf(bufs[i]);
  }
}

// --

int tcp_client() {
  Codec2 codec('|');

  struct sockaddr_in dest;
  uv_ip4_addr("127.0.0.1", 7001, &dest);

  auto loop = LoopT::defaultLoop();
  // auto loop = std::make_unique<LoopT>();
  TcpClient client(loop.get(), (const struct sockaddr *)&dest, codec);
  int r = loop->run(UV_RUN_DEFAULT);
  LOG_IF_ERROR(r);
  r = loop->close();
  LOG_IF_ERROR(r);

  return r;
}
