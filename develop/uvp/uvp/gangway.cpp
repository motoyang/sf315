#include <string>
#include <iostream>
#include <sstream>
#include <atomic>

#include <uv.h>
#include <ringbuffer.hpp>
#include <utilites.hpp>
#include <misc.hpp>
#include <req.hpp>
#include <uv.hpp>
#include <gangway.hpp>

// --

Codec::Codec(char mark) : _mark(mark) {}

BufT Codec::encode(const char* p, size_t len) {
  BufT b; b.len = 0; b.base = 0;
  if ((len > std::numeric_limits<short>::max()) || !p) {
    return b;
  }

  char buf[10] = {0};
  std::snprintf(buf, sizeof(buf), "%d%c", (int)len, _mark);
  int head = std::strlen(buf);
  
  b = allocBuf(head + len);
  memcpy(b.base, buf, len);
  memcpy(b.base + head, p, len);

  return b;
}

BufT Codec::decode(RingBuffer* ringbuffer) {
  BufT r;
  r.len = 0;
  r.base = nullptr;

  int head_len = 0;
  int body_len = 0;

  {
    char *body = ringbuffer->search(_mark);
    if (!body) {
      return r;
    }
    head_len = ringbuffer->offset(body);
  }

  {
    char header[20];
    ringbuffer->peek(header, head_len);
    header[head_len] = '\0';
    body_len = atoi(header);
    if (body_len <= 0) {
      return r;
    }
  }

  if (ringbuffer->size() < (head_len + body_len + 1)) {
    return r;
  }

  r = allocBuf(body_len);
  ringbuffer->advance(head_len + 1);
  ringbuffer->read(r.base, body_len);

  return r;
}

// --

void Business::workCallback() {
  Packet packet;
  while (_running) {
    while(_gangway._upward.try_dequeue(packet)) {
      doSomething(std::move(packet));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    // LOG_INFO << "business is doing...";
  }
}

void Business::afterWorkCallback(int status) {
  if (status < 0) {
    LOG_IF_ERROR(status);
  }
  LOG_INFO << "afterwork called.";
}

void Business::doSomething(Packet&& packet) {
  char buf[30] = {0};
  memcpy(buf, packet._buf.base, packet._buf.len);
  std::cout << packet._peer << ", " << buf << std::endl;
  // freeBuf(packet._buf);
}

Business::Business(const std::string& name, Gangway& way): _name(name), _gangway(way) {
  _work.workCallback(std::bind(&Business::workCallback, this));
  _work.afterWorkCallback(std::bind(&Business::afterWorkCallback, this, std::placeholders::_1));
}

int Business::start(LoopT* from) {
  int r = _work.queue(from);
  LOG_IF_ERROR(r);
  return r;
}

void Business::stop() {
  _running.store(false);
}

