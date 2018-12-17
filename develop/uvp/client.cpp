#include <iostream>

#include <utilites.hpp>
#include <misc.hpp>

#include <msgpack-c/msgpack.hpp>
#include <resolver.h>

#include <pp/prettyprint.h>
#include "client.h"

// --

// --

TcpConnector::TcpConnector(LoopI *loop, const struct sockaddr *dest)
    : _socket(loop), _async(loop), _timer(loop), _dest(*dest) {
  // _socket.setDefaultSize(64, 3);
  std::srand(std::time(nullptr));

  _socket.connectCallback(
      std::bind(&TcpConnector::onConnect, this, std::placeholders::_1));
  _socket.shutdownCallback(
      std::bind(&TcpConnector::onShutdown, this, std::placeholders::_1));
  _socket.closeCallback(std::bind(&TcpConnector::onClose, this));
  _socket.readCallback(std::bind(&TcpConnector::onRead, this,
                                 std::placeholders::_1, std::placeholders::_2));
  _socket.writeCallback(std::bind(&TcpConnector::onWrite, this,
                                  std::placeholders::_1, std::placeholders::_2,
                                  std::placeholders::_3));

  _timer.timerCallback(std::bind(&TcpConnector::onTimer, this));
  _async.asyncCallback(std::bind(&TcpConnector::onAsync, this));

  int r = _socket.connect(dest);
  LOG_IF_ERROR(r);
}

std::string TcpConnector::name() {
  if (_name.empty()) {
    _name = nameOfSock(AF_INET, &_socket);
  }
  return _name;
}

std::string TcpConnector::peer() {
  if (_peer.empty()) {
    _peer = nameOfPeer(AF_INET, &_socket);
  }
  return _peer;
}

void TcpConnector::notify(int tag) {
  _notifyTag = tag;
  _async.send();
}

void TcpConnector::onTimer() {
  if (!_socket.isWritable() || !_socket.isReadable()) {
    int r = _socket.reinit();
    LOG_IF_ERROR(r);
    r = _socket.connect(&_dest);
    LOG_IF_ERROR(r);
    return;
  }
}

void TcpConnector::onAsync() {
  if (_notifyTag == 1) {
    _timer.stop();
    _timer.close();

    _async.close();

    _socket.readStop();
    int r = _socket.shutdown();
    if (r) {
      LOG_IF_ERROR(r);
      _socket.close();
    }

    return;
  }

  if (!_socket.isWritable()) {
    return;
  }

  // size_t size_approx = _gangway._downward.size_approx();
  // std::cout << "size-approx: " << size_approx << std::endl;
  // std::vector<BufT> bufs;
  // bufs.reserve(size_approx);
  BufT bufs[10] = {0};
  size_t count = 0;
  while ((count = _gangway._downward.try_dequeue_bulk(bufs, COUNT_OF(bufs))) >
         0) {
    int r = _socket.write(bufs, count);
    if (r) {
      for (int i = 0; i < count; ++i) {
        freeBuf(bufs[i]);
      }
      LOG_IF_ERROR(r);
    }
  }
}

void TcpConnector::dispatch(BufT buf) {
  size_t offset = 0;
  int buf_type = 0;
  msgpack::object_handle oh = msgpack::unpack(buf.base, buf.len, offset);
  oh.get().convert(buf_type);
  if (buf_type == (int)BufType::BUF_REP_TYPE) {
    msgpack::object_handle oh2 = msgpack::unpack(buf.base, buf.len, offset);
    int token = 0;
    oh2.get().convert(token);

    std::lock_guard<std::mutex> lk(_mutex);
    if (_reqMap.find(token) != _reqMap.end()) {
      _reqMap[token] = buf;
      _cond_var.notify_all();
    } else {
      freeBuf(buf);
    }
  } else {
    // 进队列失败时，重新进队列直到进入队列成功。
    while (!_gangway._upward.try_enqueue(buf));
  }
}

void TcpConnector::makeup(const char *p, size_t len) {
  int remain = len;
  do {
    int writed = _codec.ringBuffer().write(p, remain);
    remain -= writed;
    p += writed;

    while (true) {
      // 解析出每个包
      BufT b = {0};
      if (!_codec.decode(b)) {
        break;
      }
      dispatch(b);
    }
  } while (remain > 0);
}

void TcpConnector::onConnect(int status) {
  if (status < 0) {
    LOG_IF_ERROR(status);
    return;
  }
  LOG_INFO << "client socket connected to: " << nameOfPeer(AF_INET, &_socket);
  _name = nameOfSock(AF_INET, &_socket);
  _socket.readStart();

  _timer.start(1000, 5000);
}

void TcpConnector::onShutdown(int status) {
  if (status < 0) {
    LOG_IF_ERROR(status);
  }
  LOG_INFO << "client socket shutdown.";
  _socket.close();
}

void TcpConnector::onClose() { LOG_INFO << "handle of socket closed."; }

void TcpConnector::onRead(ssize_t nread, const BufT *buf) {
  if (nread < 0) {
    LOG_IF_ERROR(nread);
    _socket.readStop();
    _socket.close();
    return;
  }

  if (nread) {
    makeup(buf->base, nread);
  }
}

void TcpConnector::onWrite(int status, BufT bufs[], int nbufs) {
  if (status < 0) {
    LOG_IF_ERROR(status);
  }

  for (int i = 0; i < nbufs; ++i) {
    freeBuf(bufs[i]);
  }
}

size_t TcpConnector::upwardDequeue(BufT bufs[], size_t nbufs) {
  return _gangway._upward.wait_dequeue_bulk_timed(
      bufs, nbufs, std::chrono::milliseconds(500));
}

int TcpConnector::downwardEnqueue(const char *p, size_t len) {
  size_t sleep_ms = _gangway._downward.size_approx() / 10;
  if (sleep_ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
  }

  BufT b = {0};
  if (!_codec.encode(b, p, len)) {
    return 0;
  }
  // 进队列失败时，重新进队列直到进入队列成功。
  while(!_gangway._downward.enqueue(b)) {
    LOG_CRIT << "enqueue faild in connector downward queue.";
  }

  int r = _async.send();
  LOG_IF_ERROR(r);
  return r;
}

// --

int netStart(LoopI *loop) {
  int r = loop->run(UV_RUN_DEFAULT);
  LOG_IF_ERROR(r);
  r = loop->close();
  LOG_IF_ERROR(r);

  LOG_INFO << "net job closed.";
  return r;
}

void f21_pair(int i, const std::string &s, float f) {
  static size_t count = 0;
  if (++count % 10000) return;
  std::cout << "f21_pair: i = " << i << ", s = " << s << ", f = " << f
            << std::endl;
}

void f22_pair(float f, const std::string &s) {
  static size_t count = 0;
  if (++count % 10000) return;
  std::cout << "f22_pair: f = " << f << ", s = " << s << std::endl;
}

void f23_pair(const std::string &s) {
  static size_t count = 0;
  if (++count % 10000) return;
  std::cout << "f23_pair: s = " << s << std::endl;
}

void f24_pair(const std::tuple<int, std::string, float, double> &t) {
  static size_t count = 0;
  if (++count % 10000) return;
  std::cout << "f24_pair: t = " << t << std::endl;
}

bool g_running = true;

int f_output(TcpConnector* client) {
  for (int i = 0; i < 10000; ++i) {
    int r = client->transmit(BufType::BUF_ECHO_TYPE, 21, 38,
                          std::string("string1"), 8.8f);
    LOG_IF_ERROR(r);

    r = client->transmit(BufType::BUF_ECHO_TYPE, 22, 0.86,
                      std::string("string2"));
    LOG_IF_ERROR(r);

    if (i % 10000 == 0) {
      int i2 = std::rand() % 100, j2 = std::rand() % 100;
      int result = 0;
      if (0 == client->request(result, 31, i2, j2)) {
        std::cout << i2 << " + " << j2 << " = " << result << std::endl;
      } else {
        std::cout << "timeout..." << std::endl;
      }
    }

    r = client->transmit(BufType::BUF_RESOLVE_TYPE, 23, 99, (double)0.1386,
                      std::string("string3"));
    LOG_IF_ERROR(r);

    r = client->transmit(BufType::BUF_ECHO_TYPE, 24, std::make_tuple(36,
                      std::string("str in tuple"), 1.88f, (double)0.248));
    LOG_IF_ERROR(r);    
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(5000));
  g_running = false;
  return 0;
}

int f_output2(TcpConnector* client) {
  for (int i = 0; i < 20000000; ++i) {
    std::string s(std::rand() % 46 + 1, '=');
    client->downwardEnqueue(s.data(), s.length());
  }

  for(;;) {
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    bufCount();
  }
  return 0;
}

int tcp_client() {
  auto resolver = std::make_unique<uvp::Resolver<int>>();
  resolver->defineFun(21, f21_pair)
      .defineFun(22, f22_pair)
      .defineFun(23, f23_pair)
      .defineFun(24, f24_pair);
  std::srand(std::time(nullptr));

  struct sockaddr_in dest;
  uv_ip4_addr("127.0.0.1", 7001, &dest);

  // auto loop = LoopT::defaultLoop();
  auto loop = std::make_unique<LoopT>();
  TcpConnector client(loop.get(), (const struct sockaddr *)&dest);
  std::thread t1(netStart, loop.get());
  std::thread t2(f_output, &client);

   while (g_running) {
    BufT b[10] = {0};
    size_t count = client.upwardDequeue(b, COUNT_OF(b));
    for (int i = 0; i < count; ++i) {
      size_t offset = 0;
      msgpack::object_handle oh =
          msgpack::unpack(b[i].base, b[i].len, offset);
      int buf_type = 0;
      oh.get().convert(buf_type);
      msgpack::object_handle oh2 =
          msgpack::unpack(b[i].base, b[i].len, offset);
      int token = 0;
      oh2.get().convert(token);
      resolver->resolve(b[i].base, b[i].len, offset);

      freeBuf(b[i]);
    }
  }

  client.notify(1);
  t1.join();
  t2.join();

  return 0;
}
