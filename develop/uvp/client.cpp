#include <iostream>

#include <uv.h>
#include <utilites.hpp>
#include <misc.hpp>
#include <uv.hpp>

#include "client.h"

// ringbuffer与msg.length的关系：
// ringbuffer.capacity() > msg.length()
// 这是一个必须的条件，否则不能parse出msg。
// BufT的长度没有要求。

// --

ParketSolver::ParketSolver(size_t size) : _buf(size), _msg(size + 1, 0) {}

char *ParketSolver::parse(char *msg, size_t msg_size, RingBuffer *buf) {
  char *r = nullptr;
  if (buf->size() == 0 || msg_size < buf->size()) {
    return r;
  }

  int head_len = 0;
  int body_len = 0;

  {
    char *body = buf->search('|');
    if (!body) {
      std::cout << "[Partial Packet] header not ready, buffer " << buf->size()
                << std::endl;
      return r;
    }
    head_len = buf->offset(body);
  }

  {
    char header[20];
    buf->peek(header, head_len);
    header[head_len] = '\0';
    body_len = atoi(header);
    if (body_len <= 0) {
      return r;
    }
  }

  if (buf->size() < (head_len + body_len + 1)) {
    std::cout << "[Partial Packet] body not ready, buffer " << buf->size()
              << std::endl;
    return r;
  }

  buf->advance(head_len + 1);
  buf->read(msg, body_len);
  msg[body_len] = '\0';

  return msg;
}

void ParketSolver::doBussiness(const char *p, size_t len) {
  int remain = len;
  do {
    int writed = _buf.write(p, remain);
    remain -= writed;
    p += writed;

    int n = 0;
    while (true) {
      char *r = parse(_msg.data(), _msg.capacity(), &_buf);
      if (!r) {
        break;
      }
      n++;

      std::cout << r << std::endl;
      if (n > _buf.capacity()) {
        std::cout << "    [Merged Packet]" << std::endl;
        exit(0);
      }
    }
  } while (remain > 0);
}

// --

TcpClient::TcpClient(LoopT *loop, const struct sockaddr *dest)
    : _socket(loop), _solver(23) {
  _socket.setDefaultSize(_solver.size() / 2, 3);

  _socket.connectCallback(
      std::bind(&TcpClient::onConnect, this, std::placeholders::_1));
  _socket.shutdownCallback(
      std::bind(&TcpClient::onShutdown, this, std::placeholders::_1));
  _socket.readCallback(std::bind(&TcpClient::onRead, this,
                                 std::placeholders::_1, std::placeholders::_2));
  _socket.writeCallback(std::bind(&TcpClient::onWrite, this,
                                  std::placeholders::_1, std::placeholders::_2,
                                  std::placeholders::_3));

  _socket.connect(dest);
}

void TcpClient::onConnect(int status) {
  if (status < 0) {
    LOG_IF_ERROR(status);
    return;
  }
  LOG_INFO << "client socket connected.";
  _socket.readStart();
}

void TcpClient::onShutdown(int status) {
  if (status < 0) {
    if (UV_ENOTCONN != status) {
      LOG_IF_ERROR(status);
      return;
    }
  }
  LOG_INFO << "client socket shutdown.";
  _socket.close();
}

void TcpClient::onClose() {
  LOG_INFO << "handle of socket closed.";
}

void TcpClient::onRead(ssize_t nread, const BufT *buf) {
  if (nread < 0) {
    if (UV_EOF == nread || UV_ECONNRESET == nread) {
      int r = _socket.shutdown();
      LOG_IF_ERROR(r);
      return;
    }
    LOG_IF_ERROR(nread);
  }
  _solver.doBussiness(buf->base, nread);

  BufT b = copyToBuf(buf->base, nread);
  int r = _socket.write(&b, 1);
  if (r) {
    freeBuf(b);
    LOG_IF_ERROR(r);
  }
}

void TcpClient::onWrite(int status, BufT bufs[], int nbufs) {
  if (status < 0) {
    LOG_IF_ERROR(status);
  }

  for (int i = 0; i < nbufs; ++i) {
    freeBuf(bufs[i]);
    LOG_INFO << "write " << bufs[i].len << " bytes.";
  }
}

// --

int tcp_client(LoopT *loop) {
  LOG_IF_ERROR(-107);

  struct sockaddr_in dest;
  uv_ip4_addr("127.0.0.1", 7001, &dest);

  TcpClient client(loop, (const struct sockaddr *)&dest);
  int r = loop->run(UV_RUN_DEFAULT);
  LOG_IF_ERROR(r);
  return r;
}
