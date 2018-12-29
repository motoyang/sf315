#pragma once

#include <functional>

#include <details/types.hpp>
#include <details/misc.hpp>
#include <details/eventloop.hpp>

// --

namespace uvp {

template <typename T> struct AnyReq {
  T req;
  uv::BufT buf;
};

template <typename T>
AnyReq<T> *allocAnyReq(const uv::BufT bufs[], size_t nbufs) {
  auto req = (AnyReq<T> *)malloc(sizeof(AnyReq<T>));
  if (!req) {
    LOG_IF_ERROR_EXIT(UV_ENOMEM);
  }

  size_t len = sizeof(uv::BufT) * nbufs;
  if (len) {
    req->buf.base = (char *)malloc(len);
    if (!req->buf.base) {
      LOG_IF_ERROR_EXIT(UV_ENOMEM);
    }

    // 注意：这个BufT的len与base的长度不同！
    req->buf.len = nbufs;
    memcpy(req->buf.base, bufs, len);
  } else {
    req->buf.len = 0;
    req->buf.base = nullptr;
  }
  return req;
}

template <typename T> void freeAnyReq(AnyReq<T> *req) {
  UVP_ASSERT(req);

  if (req->buf.base) {
    free(req->buf.base);
  }
  free(req);
}

// --

class Handle {
public:
  using Type = uv::HandleType;

  using AllocCallback = std::function<void(size_t, uv::BufT *)>;
  using CloseCallback = std::function<void(Handle *)>;

protected:
  struct Impl {
    void *_data = nullptr;
    uv::BufT _buffer = {0};

    AllocCallback _allocCallback;
    CloseCallback _closeCallback;

    virtual ~Impl() {
      if (_buffer.base) {
        freeBuf(_buffer);
      }
    }
  };
  Impl _impl;

  static void alloc_callback(uv::HandleT *handle, size_t suggested_size,
                             uv::BufT *buf) {
    auto p = (Handle *)uv_handle_get_data(handle);
    if (p->_impl._allocCallback) {
      p->_impl._allocCallback(suggested_size, buf);
    } else {
      if (!p->_impl._buffer.base) {
        p->_impl._buffer = allocBuf(suggested_size);
      }
      *buf = p->_impl._buffer;
    }
  }

  static void close_callback(uv::HandleT *handle) {
    auto p = (Handle *)uv_handle_get_data(handle);
    if (p->_impl._closeCallback) {
      p->_impl._closeCallback(p);
    }
  }

public:
  virtual uv::HandleT *handle() const = 0;

  static size_t size(Type t) { return uv_handle_size(t); }

  static const char *typeName(Type t) { return uv_handle_type_name(t); }

  Handle() {}

  virtual ~Handle() {}

  bool isActive() const { return uv_is_active(handle()); }

  bool isClosing() const { return uv_is_closing(handle()); }

  void close(const CloseCallback &cb) {
    if (!cb) {
      uv_close(handle(), nullptr);
    } else {
      _impl._closeCallback = cb;
      uv_close(handle(), close_callback);
    }
  }

  void ref() { uv_ref(handle()); }

  void unref() { uv_unref(handle()); }

  bool hasRef() const { return uv_has_ref(handle()); }

  // This function works for TCP, pipe and UDP handles on Unix and for TCP and
  // UDP handles on Windows.
  int sendBufferSize(int *value) {
    return uv_send_buffer_size(handle(), value);
  }

  // This function works for TCP, pipe and UDP handles on Unix and for TCP and
  // UDP handles on Windows.
  int recvBufferSize(int *value) {
    return uv_recv_buffer_size(handle(), value);
  }

  // The following handles are supported: TCP, pipes, TTY, UDP and poll. Passing
  // any other handle type will fail with UV_EINVAL.
  int fileno(uv::OsFdT *fd) { return uv_fileno(handle(), fd); }

  Loop *loop() const {
    uv_loop_t *loop = uv_handle_get_loop(handle());
    return (Loop *)uv_loop_get_data(loop);
  }

  void *data() const { return _impl._data; }

  void *data(void *data) {
    _impl._data = data;
    return data;
  }

  Type type() const { return uv_handle_get_type(handle()); }
};
/*
class HandleObject : public Handle {
  mutable uv::HandleT _handle;

protected:
  virtual uv::HandleT *handle() const override { return &_handle; }

public:
  HandleObject() { uv_handle_set_data(handle(), this); }
  virtual ~HandleObject() {}
};
*/
// --

class Idle : public Handle {
public:
  using IdleCallback = std::function<void(Idle *)>;

protected:
  struct Impl {
    IdleCallback _idleCallback;
  };
  Impl _impl;

  static void idle_callback(uv::IdleT *idle) {
    auto p = (Idle *)uv_handle_get_data((uv::HandleT *)idle);
    if (p->_impl._idleCallback) {
      p->_impl._idleCallback(p);
    } else {
      UVP_ASSERT(false);
    }
  }

public:
  virtual uv::IdleT *idle() const = 0;

  Idle() {}
  virtual ~Idle() {}

  int start(const IdleCallback &cb) {
    UVP_ASSERT(cb);
    _impl._idleCallback = cb;
    int r = uv_idle_start(idle(), idle_callback);
    LOG_IF_ERROR(r);
    return r;
  }

  int stop() {
    int r = uv_idle_stop(idle());
    LOG_IF_ERROR(r);
    return r;
  }
};

class IdleObject : public Idle {
  mutable uv::IdleT _idle;

public:
  virtual uv::HandleT *handle() const override { return (uv::HandleT *)&_idle; }

  virtual uv::IdleT *idle() const override { return &_idle; }

  IdleObject(Loop *loop) {
    int r = uv_idle_init(loop->loop(), idle());
    LOG_IF_ERROR_EXIT(r);
    uv_handle_set_data(handle(), this);
  }
  virtual ~IdleObject() {}
};

// --

class Timer : public Handle {
public:
  using TimerCallback = std::function<void(Timer *)>;

protected:
  struct Impl {
    TimerCallback _timerCallback;
  };
  Impl _impl;

  static void timer_callback(uv::TimerT *timer) {
    auto p = (Timer *)uv_handle_get_data((uv::HandleT *)timer);
    if (p->_impl._timerCallback) {
      p->_impl._timerCallback(p);
    } else {
      UVP_ASSERT(false);
    }
  }

public:
  virtual uv_timer_t *timer() const = 0;

  Timer() {}
  virtual ~Timer() {}

  int start(const TimerCallback &cb, UnsignedLong timeout,
            UnsignedLong repeat) {
    UVP_ASSERT(cb);
    _impl._timerCallback = cb;
    int r = uv_timer_start(timer(), timer_callback, timeout, repeat);
    LOG_IF_ERROR(r);
    return r;
  }

  int stop() {
    int r = uv_timer_stop(timer());
    LOG_IF_ERROR(r);
    return r;
  }

  int again() {
    int r = uv_timer_again(timer());
    LOG_IF_ERROR(r);
    return r;
  }

  void repeat(uint64_t repeat) { uv_timer_set_repeat(timer(), repeat); }

  UnsignedLong repeat() const { return uv_timer_get_repeat(timer()); }
};

class TimerObject : public Timer {
  mutable uv::TimerT _timer;

public:
  virtual uv::HandleT *handle() const override {
    return (uv::HandleT *)&_timer;
  }
  virtual uv::TimerT *timer() const override { return &_timer; }

  TimerObject(Loop *loop) {
    int r = uv_timer_init(loop->loop(), &_timer);
    LOG_IF_ERROR_EXIT(r);

    uv_handle_set_data(handle(), this);
  }
  virtual ~TimerObject() {}
};

// --

class Stream : public Handle {
public:
  using ConnectCallback = std::function<void(Stream *, int)>;
  using ReadCallback = std::function<void(Stream *, ssize_t, const uv::BufT *)>;
  using WriteCallback = std::function<void(Stream *, int, uv::BufT[], int)>;
  using ShutdownCallback = std::function<void(Stream *, int)>;
  using ConnectionCallback = std::function<void(Stream *, int)>;

protected:
  struct Impl {
    ConnectCallback _connectCallback;
    ReadCallback _readCallback;
    WriteCallback _writeCallback;
    ShutdownCallback _shutdownCallback;
    ConnectionCallback _connectionCallback;
  };
  Impl _impl;

  static void connect_callback(uv::ConnectT *req, int status) {
    uv::StreamT *h = req->handle;
    auto p = (Stream *)uv_handle_get_data((uv::HandleT *)h);

    if (p->_impl._connectCallback) {
      p->_impl._connectCallback(p, status);
    }

    // 释放在connect()中申请的req
    freeAnyReq((AnyReq<uv::ConnectT> *)req);
  }

  static void read_callback(uv::StreamT *stream, ssize_t nread,
                            const uv::BufT *buf) {
    auto p = (Stream *)uv_handle_get_data((uv::HandleT *)stream);
    if (p->_impl._readCallback) {
      p->_impl._readCallback(p, nread, buf);
    }
    /*
        //
       释放在read时分配的memory，这个memory是通过Handle的allocCallback分配的。
        if (p->Handle::_impl._freeCallback) {
          UVP_ASSERT(p->Handle::_impl._allocCallback);
          p->Handle::_impl._freeCallback(*buf);
        }
    */
  }

  static void write_callback(uv::WriteT *req, int status) {
    uv::StreamT *h = req->handle;
    auto p = (Stream *)uv_handle_get_data((uv::HandleT *)h);
    auto anyReq = (AnyReq<uv::WriteT> *)req;

    // 在回调函数中，删除buf.base是用户的责任。
    if (p->_impl._writeCallback) {
      p->_impl._writeCallback(p, status, (uv::BufT *)anyReq->buf.base,
                              anyReq->buf.len);
    }

    // 删除在write函数中申请的memory。
    freeAnyReq(anyReq);
  }

  static void shutdown_callback(uv::ShutdownT *req, int status) {
    uv::StreamT *h = req->handle;
    auto p = (Stream *)uv_handle_get_data((uv::HandleT *)h);

    if (p->_impl._shutdownCallback) {
      p->_impl._shutdownCallback(p, status);
    }

    // 释放在shutdown()中申请的req
    freeAnyReq((AnyReq<uv::ShutdownT> *)req);
  }

  static void connection_callback(uv::StreamT *server, int status) {
    auto p = (Stream *)uv_handle_get_data((uv::HandleT *)server);
    if (p->_impl._connectionCallback) {
      p->_impl._connectionCallback(p, status);
    }
  }

public:
  virtual uv_stream_t *stream() const = 0;

  Stream() {}
  virtual ~Stream() {}

  int shutdown(const ShutdownCallback &cb) {
    UVP_ASSERT(cb);
    _impl._shutdownCallback = cb;

    auto req = allocAnyReq<uv_shutdown_t>(nullptr, 0);
    int r = uv_shutdown(&req->req, stream(), shutdown_callback);
    if (r) {
      freeAnyReq(req);
      LOG_IF_ERROR(r);
    }
    return r;
  }

  int listen(int backlog, const ConnectionCallback &cb) {
    UVP_ASSERT(cb);
    _impl._connectionCallback = cb;

    int r = uv_listen(stream(), backlog, connection_callback);
    LOG_IF_ERROR(r);
    return r;
  }

  int accept(Stream *client) {
    int r = uv_accept(stream(), client->stream());
    LOG_IF_ERROR(r);
    return r;
  }

  int readStart(const Handle::AllocCallback &alloc_cb,
                const ReadCallback &read_cb) {
    UVP_ASSERT(read_cb);
    _impl._readCallback = read_cb;
    Handle::_impl._allocCallback = alloc_cb;

    int r = uv_read_start(stream(), Handle::alloc_callback, read_callback);
    LOG_IF_ERROR(r);
    return r;
  }

  int readStop() {
    int r = uv_read_stop(stream());
    LOG_IF_ERROR(r);
    return r;
  }

  int write(const uv::BufT bufs[], unsigned int nbufs,
            const WriteCallback &cb) {
    UVP_ASSERT(cb);
    _impl._writeCallback = cb;

    auto req = allocAnyReq<uv::WriteT>(bufs, nbufs);
    int r = uv_write((uv::WriteT *)req, stream(), bufs, nbufs, write_callback);
    if (r) {
      freeAnyReq(req);
      LOG_IF_ERROR(r);
    }
    return r;
  }

  int write2(const uv::BufT bufs[], unsigned int nbufs, Stream *sendstream,
             const WriteCallback &cb) {
    UVP_ASSERT(cb);
    _impl._writeCallback = cb;

    auto req = allocAnyReq<uv::WriteT>(bufs, nbufs);
    int r = uv_write2((uv::WriteT *)req, stream(), bufs, nbufs,
                      sendstream->stream(), write_callback);
    if (r) {
      freeAnyReq(req);
      LOG_IF_ERROR(r);
    }
    return r;
  }

  int tryWrite(uv::BufT bufs[], unsigned int nbufs) {
    int r = uv_try_write(stream(), bufs, nbufs);
    LOG_IF_ERROR(r);
    return r;
  }

  int isReadable() const { return uv_is_readable(stream()); }

  int isWritable() const { return uv_is_writable(stream()); }

  int setBlocking(int blocking) {
    int r = uv_stream_set_blocking(stream(), blocking);
    LOG_IF_ERROR(r);
    return r;
  }

  int getWriteQueueSize() const {
    return uv_stream_get_write_queue_size(stream());
  }
};

class StreamObject : public Stream {
  friend class Stream;
  mutable uv::StreamT _stream;

public:
  virtual uv::HandleT *handle() const override {
    return (uv::HandleT *)&_stream;
  }

  virtual uv::StreamT *stream() const override { return &_stream; }

  StreamObject() { uv_handle_set_data((uv_handle_t *)&_stream, this); }
  virtual ~StreamObject() {}
};

// --

class Pipe : public Stream {
public:
  virtual uv::PipeT *pipe() const = 0;

  int open(uv::File file) {
    int r = uv_pipe_open(pipe(), file);
    LOG_IF_ERROR(r);
    return r;
  }

  int bind(const char *name) {
    int r = uv_pipe_bind(pipe(), name);
    LOG_IF_ERROR(r);
    return r;
  }

  void connect(const char *name, const Stream::ConnectCallback &cb) {
    UVP_ASSERT(cb);
    Stream::_impl._connectCallback = cb;

    auto req = allocAnyReq<uv::ConnectT>(nullptr, 0);
    uv_pipe_connect(&req->req, pipe(), name, Stream::connect_callback);
  }

  int getSockname(char *buffer, size_t *size) const {
    int r = uv_pipe_getsockname(pipe(), buffer, size);
    LOG_IF_ERROR(r);
    return r;
  }

  int getPeername(char *buffer, size_t *size) const {
    int r = uv_pipe_getpeername(pipe(), buffer, size);
    LOG_IF_ERROR(r);
    return r;
  }

  void pendingInstances(int count) { uv_pipe_pending_instances(pipe(), count); }

  int pendingCount() {
    int r = uv_pipe_pending_count(pipe());
    LOG_IF_ERROR(r);
    return r;
  }

  Handle::Type pendingType() { return uv_pipe_pending_type(pipe()); }

  int chmod(int flags) {
    int r = uv_pipe_chmod(pipe(), flags);
    LOG_IF_ERROR(r);
    return r;
  }
};

class PipeObject : public Pipe {
  mutable uv::PipeT _pipe;

public:
  virtual uv::HandleT *handle() const override { return (uv::HandleT *)&_pipe; }
  virtual uv::StreamT *stream() const override { return (uv::StreamT *)&_pipe; }
  virtual uv::PipeT *pipe() const override { return &_pipe; }

  PipeObject(Loop *loop, int ipc) {
    int r = uv_pipe_init(loop->loop(), &_pipe, ipc);
    LOG_IF_ERROR_EXIT(r);

    uv_handle_set_data(handle(), this);
  }
  ~PipeObject() {}
};

// --

class Tcp : public Stream {
public:
  virtual uv_tcp_t *tcp() const = 0;

  int open(uv::OsSockT sock) {
    int r = uv_tcp_open(tcp(), sock);
    LOG_IF_ERROR(r);
    return r;
  }

  int nodelay(int enable) {
    int r = uv_tcp_nodelay(tcp(), enable);
    LOG_IF_ERROR(r);
    return r;
  }

  int keepalive(int enable, unsigned int delay) {
    int r = uv_tcp_keepalive(tcp(), enable, delay);
    LOG_IF_ERROR(r);
    return r;
  }

  int simultaneousAcepts(int enable) {
    int r = uv_tcp_simultaneous_accepts(tcp(), enable);
    LOG_IF_ERROR(r);
    return r;
  }

  int bind(const struct sockaddr *addr, unsigned int flags) {
    int r = uv_tcp_bind(tcp(), addr, flags);
    LOG_IF_ERROR(r);
    return r;
  }

  int getsockname(struct sockaddr *name, int *namelen) {
    int r = uv_tcp_getsockname(tcp(), name, namelen);
    LOG_IF_ERROR(r);
    return r;
  }

  int getpeername(struct sockaddr *name, int *namelen) {
    int r = uv_tcp_getpeername(tcp(), name, namelen);
    LOG_IF_ERROR(r);
    return r;
  }

  int connect(const struct sockaddr *addr, const Stream::ConnectCallback &cb) {
    UVP_ASSERT(cb);
    Stream::_impl._connectCallback = cb;

    auto req = allocAnyReq<uv::ConnectT>(nullptr, 0);
    int r = uv_tcp_connect(&req->req, tcp(), addr, Stream::connect_callback);
    if (r) {
      // 如果失败，释放申请的req。如果成功，在connect_cb中释放req。
      freeAnyReq(req);
      LOG_IF_ERROR(r);
    }
    return r;
  }

  // extend functions
  int reinit() {
    int r = uv_tcp_init(loop()->loop(), tcp());
    LOG_IF_ERROR(r);
    uv_handle_set_data(handle(), this);
    return r;
  }
};

class TcpObject : public Tcp {
  mutable uv::TcpT _tcp;

public:
  virtual uv::HandleT *handle() const override { return (uv::HandleT *)&_tcp; }
  virtual uv::StreamT *stream() const override { return (uv::StreamT *)&_tcp; }
  virtual uv::TcpT *tcp() const override { return &_tcp; }

  TcpObject(Loop *loop) {
    int r = uv_tcp_init(loop->loop(), &_tcp);
    LOG_IF_ERROR_EXIT(r);

    uv_handle_set_data(handle(), this);
  }
  TcpObject(Loop *loop, unsigned int flags) {
    int r = uv_tcp_init_ex(loop->loop(), &_tcp, flags);
    LOG_IF_ERROR_EXIT(r);

    uv_handle_set_data(handle(), this);
  }
  virtual ~TcpObject() {}
};

// --

class Tty : public Stream {
public:
  using TtyMode = uv::TtyMode;

  virtual uv::TtyT *tty() const = 0;

  static int resetMode() {
    int r = uv_tty_reset_mode();
    LOG_IF_ERROR(r);
    return r;
  }

  int setMode(TtyMode mode) {
    int r = uv_tty_set_mode(tty(), mode);
    LOG_IF_ERROR(r);
    return r;
  }

  int getWinsize(int *width, int *height) {
    int r = uv_tty_get_winsize(tty(), width, height);
    LOG_IF_ERROR(r);
    return r;
  }
};

class TtyObject : public Tty {
  mutable uv::TtyT _tty;

public:
  virtual uv::HandleT *handle() const override { return (uv::HandleT *)&_tty; }
  virtual uv::StreamT *stream() const override { return (uv::StreamT *)&_tty; }
  virtual uv::TtyT *tty() const override { return &_tty; }

  TtyObject(Loop *loop, uv::File fd, int unused) {
    int r = uv_tty_init(loop->loop(), &_tty, fd, unused);
    LOG_IF_ERROR_EXIT(r);

    uv_handle_set_data(handle(), this);
  }
  virtual ~TtyObject() {}
};

// --

class Udp : public Handle {
public:
  using MemberShip = uv::Membership;

  using SendCallback = std::function<void(Udp *, int, uv::BufT *, int)>;
  using RecvCallback = std::function<void(Udp *, ssize_t, const uv::BufT *,
                                          const struct sockaddr *, unsigned)>;

protected:
  struct Impl {
    SendCallback _sendCallback;
    RecvCallback _recvCallback;
  };
  Impl _impl;

  static void send_callback(uv_udp_send_t *req, int status) {
    uv::UdpT *h = req->handle;
    auto p = (Udp *)uv_handle_get_data((uv::HandleT *)h);
    auto anyReq = (AnyReq<uv::UdpSendT> *)req;

    if (p->_impl._sendCallback) {
      p->_impl._sendCallback(p, status, (uv::BufT *)anyReq->buf.base,
                             anyReq->buf.len);
    }

    // 删除在send函数中申请的memory
    freeAnyReq(anyReq);
  }

  static void recv_callback(uv::UdpT *handle, ssize_t nread,
                            const uv::BufT *buf, const struct sockaddr *addr,
                            unsigned flags) {
    auto p = (Udp *)uv_handle_get_data((uv::HandleT *)handle);
    if (p->_impl._recvCallback) {
      p->_impl._recvCallback(p, nread, buf, addr, flags);
    }
    /*
        //
       释放在recv时分配的memory，这个memory是通过Handle的allocCallback分配的。
        if (p->Handle::_impl._freeCallback) {
          UVP_ASSERT(p->Handle::_impl._allocCallback);
          p->Handle::_impl._freeCallback(*buf);
        }
    */
  }

public:
  virtual uv_udp_t *udp() const = 0;

  int open(uv::OsSockT sock) {
    int r = uv_udp_open(udp(), sock);
    LOG_IF_ERROR(r);
    return r;
  }

  int bind(const struct sockaddr *addr, unsigned int flags) {
    int r = uv_udp_bind(udp(), addr, flags);
    LOG_IF_ERROR(r);
    return r;
  }

  int getsockname(struct sockaddr *name, int *namelen) const {
    int r = uv_udp_getsockname(udp(), name, namelen);
    LOG_IF_ERROR(r);
    return r;
  }

  int setMembership(const char *multicast_addr, const char *interface_addr,
                    MemberShip membership) {
    int r = uv_udp_set_membership(udp(), multicast_addr, interface_addr,
                                  membership);
    LOG_IF_ERROR(r);
    return r;
  }

  int setMulticastLoop(int on) {
    int r = uv_udp_set_multicast_loop(udp(), on);
    LOG_IF_ERROR(r);
    return r;
  }
  int setMulticastTtl(int ttl) {
    int r = uv_udp_set_multicast_ttl(udp(), ttl);
    LOG_IF_ERROR(r);
    return r;
  }

  int setMulticastInterface(const char *interface_addr) {
    int r = uv_udp_set_multicast_interface(udp(), interface_addr);
    LOG_IF_ERROR(r);
    return r;
  }

  int setBroadcast(int on) {
    int r = uv_udp_set_broadcast(udp(), on);
    LOG_IF_ERROR(r);
    return r;
  }

  int setTtl(int ttl) {
    int r = uv_udp_set_ttl(udp(), ttl);
    LOG_IF_ERROR(r);
    return r;
  }

  int send(const uv::BufT bufs[], unsigned int nbufs,
           const struct sockaddr *addr, const SendCallback &cb) {
    UVP_ASSERT(cb);
    _impl._sendCallback = cb;

    auto req = allocAnyReq<uv::UdpSendT>(bufs, nbufs);
    int r = uv_udp_send((uv::UdpSendT *)req, udp(), bufs, nbufs, addr,
                        send_callback);
    if (r) {
      freeAnyReq(req);
      LOG_IF_ERROR(r);
    }
    return r;
  }

  int trySend(const uv::BufT bufs[], unsigned int nbufs,
              const struct sockaddr *addr) {
    int r = uv_udp_try_send(udp(), bufs, nbufs, addr);
    LOG_IF_ERROR(r);
    return r;
  }

  int recvStart(Stream::AllocCallback alloc_cb, RecvCallback recv_cb) {
    UVP_ASSERT(recv_cb);
    _impl._recvCallback = recv_cb;
    Handle::_impl._allocCallback = alloc_cb;

    int r = uv_udp_recv_start(udp(), Handle::alloc_callback, recv_callback);
    LOG_IF_ERROR(r);
    return r;
  }

  int recvStop() {
    int r = uv_udp_recv_stop(udp());
    LOG_IF_ERROR(r);
    return r;
  }

  int getSendQueueSize() const {
    int r = uv_udp_get_send_queue_size(udp());
    LOG_IF_ERROR(r);
    return r;
  }

  int getSendQueueCount() const {
    int r = uv_udp_get_send_queue_count(udp());
    LOG_IF_ERROR(r);
    return r;
  }
};

class UdpObject : public Udp {
  mutable uv::UdpT _udp;

public:
  virtual uv::HandleT *handle() const { return (uv::HandleT *)&_udp; }
  virtual uv::UdpT *udp() const { return &_udp; }

  UdpObject(Loop *loop) {
    int r = uv_udp_init(loop->loop(), &_udp);
    LOG_IF_ERROR_EXIT(r);

    uv_handle_set_data(handle(), this);
  }
  UdpObject(Loop *loop, unsigned int flags) {
    int r = uv_udp_init_ex(loop->loop(), &_udp, flags);
    LOG_IF_ERROR_EXIT(r);

    uv_handle_set_data(handle(), this);
  }
  virtual ~UdpObject() {}
};

// --

class Async : public Handle {
public:
  using AsyncCallback = std::function<void(Async *)>;

protected:
  struct Impl {
    AsyncCallback _asyncCallback;
  };
  Impl _impl;

  static void async_callback(uv::AsyncT *handle) {
    auto p = (Async *)uv_handle_get_data((uv::HandleT *)handle);
    if (p->_impl._asyncCallback) {
      p->_impl._asyncCallback(p);
    }
  }

public:
  virtual uv_async_t *async() const = 0;

  int send() {
    int r = uv_async_send(async());
    LOG_IF_ERROR(r);
    return r;
  }
};

class AsyncObject : public Async {
  mutable uv::AsyncT _async;

public:
  virtual uv::HandleT *handle() const { return (uv::HandleT *)&_async; }
  virtual uv::AsyncT *async() const { return &_async; }

  AsyncObject(Loop *loop, const Async::AsyncCallback &cb) {
    UVP_ASSERT(cb);
    Async::_impl._asyncCallback = cb;

    int r = uv_async_init(loop->loop(), &_async, async_callback);
    LOG_IF_ERROR_EXIT(r);

    uv_handle_set_data(handle(), this);
  }
  virtual ~AsyncObject() {}
};

// --

class Signal : public Handle {
public:
  using SignalCallback = std::function<void(Signal *, int)>;

protected:
  struct Impl {
    SignalCallback _signalCallback;
  };
  Impl _impl;

  static void signal_callback(uv::SignalT *handle, int signum) {
    auto p = (Signal *)uv_handle_get_data((uv::HandleT *)handle);
    if (p->_impl._signalCallback) {
      p->_impl._signalCallback(p, signum);
    }
  }

public:
  virtual uv_signal_t *signal() const = 0;

  int start(const SignalCallback &cb, int signum) {
    UVP_ASSERT(cb);
    _impl._signalCallback = cb;

    int r = uv_signal_start(signal(), signal_callback, signum);
    LOG_IF_ERROR(r);
    return r;
  }

  int startOneshort(const SignalCallback &cb, int signum) {
    UVP_ASSERT(cb);
    _impl._signalCallback = cb;

    int r = uv_signal_start_oneshot(signal(), signal_callback, signum);
    LOG_IF_ERROR(r);
    return r;
  }

  int stop() {
    int r = uv_signal_stop(signal());
    LOG_IF_ERROR(r);
    return r;
  }
};

class SignalObject : public Signal {
  mutable uv::SignalT _signal;

public:
  virtual uv::HandleT *handle() const override {
    return (uv::HandleT *)&_signal;
  }
  virtual uv::SignalT *signal() const override { return &_signal; }

  SignalObject(Loop *loop) {
    int r = uv_signal_init(loop->loop(), &_signal);
    LOG_IF_ERROR_EXIT(r);

    uv_handle_set_data(handle(), this);
  }
  virtual ~SignalObject() {}
};

// --

class Process : public Handle {
public:
  using Pid = uv::PidT;
  using Uid = uv::UidT;
  using Gid = uv::GidT;
  using StdioContainer = uv::StdioContainerT;

  using Callback =
      std::function<void(Process *, int64_t exit_status, int term_signal)>;

  struct Options {
    Callback exit_cb; /* Called after the process exits. */
    const char *file; /* Path to program to execute. */
    char **args;
    char **env;
    const char *cwd;
    unsigned int flags;
    int stdio_count;
    StdioContainer* stdio;
    Uid uid;
    Gid gid;
  };

protected:
  struct Impl {
    Callback _callback;
  };
  Impl _impl;

  static void callback(uv::ProcessT* handle, int64_t exit_status, int term_signal) {
    auto p = (Process*)uv_handle_get_data((uv::HandleT *)handle);
    if (p->_impl._callback) {
      p->_impl._callback(p, exit_status, term_signal);
    } else {
      UVP_ASSERT(false);
    }
  }

  static void disableStdioInheritance() { uv_disable_stdio_inheritance(); }

  static int kill(int pid, int signum) {
    int r = uv_kill(pid, signum);
    LOG_IF_ERROR(r);
    return r;
  }

public:
  virtual uv_process_t *process() const = 0;

  int spawn(Loop *loop, const Options *options) {
    _impl._callback = options->exit_cb;

    uv::ProcessOptionsT opt = {0};
    opt.args = options->args;
    opt.cwd = options->cwd;
    opt.env = options->env;
    opt.exit_cb = options->exit_cb? Process::callback: nullptr;
    opt.file = options->file;
    opt.flags = options->flags;
    opt.gid = options->gid;
    opt.stdio = options->stdio;
    opt.stdio_count = options->stdio_count;
    opt.uid = options->uid;

    int r = uv_spawn(loop->loop(), process(), &opt);
    LOG_IF_ERROR(r);
    return r;
  }

  int kill(int signum) {
    int r = uv_process_kill(process(), signum);
    LOG_IF_ERROR(r);
    return r;
  }

  Pid getPid() const { return uv_process_get_pid(process()); }
};

class ProcessObject : public Process {
  mutable uv::ProcessT _process;

public:
  virtual uv::HandleT *handle() const override {
    return (uv::HandleT *)&_process;
  }
  virtual uv::ProcessT *process() const override { return &_process; }

  ProcessObject() { uv_handle_set_data(handle(), this); }
  virtual ~ProcessObject(){};
};

// --

class FsEvent : public Handle {
public:
  using Callback = std::function<void(FsEvent *, const char *, int, int)>;

protected:
  struct Impl {
    Callback _callback;
  };
  Impl _impl;

  static void callback(uv::FsEventT *handle, const char *filename, int events,
                       int status) {
    auto p = (FsEvent *)uv_handle_get_data((uv::HandleT *)handle);
    if (p->_impl._callback) {
      p->_impl._callback(p, filename, events, status);
    }
  }

public:
  virtual uv::FsEventT *fsEvent() const = 0;

  int start(const Callback &cb, const char *path, unsigned int flags) {
    UVP_ASSERT(cb);
    _impl._callback = cb;

    int r = uv_fs_event_start(fsEvent(), callback, path, flags);
    LOG_IF_ERROR(r);
    return r;
  }

  int stop() {
    int r = uv_fs_event_stop(fsEvent());
    LOG_IF_ERROR(r);
    return r;
  }

  int getPath(std::string &path) {
    path.resize(1024);
    size_t len = path.length();
    int r = uv_fs_event_getpath(fsEvent(), path.data(), &len);
    if (r == UV_ENOBUFS) {
      path.resize(len);
      r = uv_fs_event_getpath(fsEvent(), path.data(), &len);
    }
    path.at(len) = 0;
    LOG_IF_ERROR(r);
    return r;
  }
};

class FsEventObject : public FsEvent {
  mutable uv::FsEventT _fsEvent;

public:
  virtual uv::HandleT *handle() const override {
    return (uv::HandleT *)&_fsEvent;
  }
  virtual uv::FsEventT *fsEvent() const override { return &_fsEvent; }

  FsEventObject(Loop *loop) {
    int r = uv_fs_event_init(loop->loop(), &_fsEvent);
    LOG_IF_ERROR_EXIT(r);

    uv_handle_set_data(handle(), this);
  }
  virtual ~FsEventObject() {}
};

} // namespace uvp
