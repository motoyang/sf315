#pragma once

#include <functional>

#include "types.hpp"
#include "misc.hpp"
#include "eventloop.hpp"

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
    UVP_LOG_ERROR_EXIT(UV_ENOMEM);
  }

  size_t len = sizeof(uv::BufT) * nbufs;
  if (len) {
    req->buf.base = (char *)malloc(len);
    if (!req->buf.base) {
      UVP_LOG_ERROR_EXIT(UV_ENOMEM);
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

  Handle() {}

public:
  virtual ~Handle() {}
  
  virtual uv::HandleT *handle() const = 0;

  static size_t size(Type t) { return uv_handle_size(t); }

  static const char *typeName(Type t) { return uv_handle_type_name(t); }

  bool isActive() const { return uv_is_active(handle()); }

  bool isClosing() const { return uv_is_closing(handle()); }

  void close(const CloseCallback &cb) {
    _impl._closeCallback = cb;
    uv_close(handle(), _impl._closeCallback ? close_callback : nullptr);
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
    uv::LoopT *loop = uv_handle_get_loop(handle());
    return (Loop *)uv_loop_get_data(loop);
  }

  void *data() const { return _impl._data; }

  void *data(void *data) {
    _impl._data = data;
    return data;
  }

  Type type() const { return uv_handle_get_type(handle()); }

  // extend functions
  void allocCallback(const AllocCallback &cb) { _impl._allocCallback = cb; }
  AllocCallback allocCallback() const { return _impl._allocCallback; }
  void closeCallback(const CloseCallback &cb) { _impl._closeCallback = cb; }
  CloseCallback closeCallback() const { return _impl._closeCallback; }
  void close() {
    uv_close(handle(), _impl._closeCallback ? close_callback : nullptr);
  }
};

// --

class Prepare : public Handle {
public:
  using PrepareCallback = std::function<void(Prepare *handle)>;

protected:
  struct Impl {
    PrepareCallback _prepareCallback;
  };
  Impl _impl;

  static void prepare_callback(uv::PrepareT *handle) {
    auto p = (Prepare *)uv_handle_get_data((uv::HandleT *)handle);
    if (p->_impl._prepareCallback) {
      p->_impl._prepareCallback(p);
    } else {
      UVP_ASSERT(false);
    }
  }

  Prepare() {}

public:
  virtual ~Prepare() {}

  virtual uv::PrepareT *prepare() const = 0;

  int start(const PrepareCallback &cb) {
    UVP_ASSERT(cb);
    _impl._prepareCallback = cb;
    int r = uv_prepare_start(prepare(), Prepare::prepare_callback);
    UVP_LOG_ERROR(r);
    return r;
  }

  int stop() {
    int r = uv_prepare_stop(prepare());
    UVP_LOG_ERROR(r);
    return r;
  }

  // extend functions
  PrepareCallback startCallback() const { return _impl._prepareCallback; }
  void startCallback(const PrepareCallback &cb) {
    UVP_ASSERT(cb);
    _impl._prepareCallback = cb;
  }
  int start() {
    int r = uv_prepare_start(prepare(), Prepare::prepare_callback);
    UVP_LOG_ERROR(r);
    return r;
  }
};

class PrepareObject : public Prepare {
  mutable uv::PrepareT _prepare;

public:
  virtual uv::HandleT *handle() const override {
    return (uv::HandleT *)&_prepare;
  }

  virtual uv::PrepareT *prepare() const override { return &_prepare; }

  PrepareObject(Loop *loop) {
    int r = uv_prepare_init(loop->loop(), prepare());
    UVP_LOG_ERROR_EXIT(r);
    uv_handle_set_data(handle(), this);
  }
  virtual ~PrepareObject() {}
};

// --

class Check : public Handle {
public:
  using CheckCallback = std::function<void(Check *)>;

protected:
  struct Impl {
    CheckCallback _checkCallback;
  };
  Impl _impl;

  static void check_callback(uv::CheckT *handle) {
    auto p = (Check *)uv_handle_get_data((uv::HandleT *)handle);
    if (p->_impl._checkCallback) {
      p->_impl._checkCallback(p);
    } else {
      UVP_ASSERT(false);
    }
  }

  Check() {}

public:
  virtual ~Check() {}

  virtual uv::CheckT *check() const = 0;

  int start(const CheckCallback &cb) {
    UVP_ASSERT(cb);
    _impl._checkCallback = cb;
    int r = uv_check_start(check(), Check::check_callback);
    UVP_LOG_ERROR(r);
    return r;
  }

  int stop() {
    int r = uv_check_stop(check());
    UVP_LOG_ERROR(r);
    return r;
  }

  // extend functions
  CheckCallback startCallback() const { return _impl._checkCallback; }
  void startCallback(const CheckCallback &cb) {
    UVP_ASSERT(cb);
    _impl._checkCallback = cb;
  }
  int start() {
    int r = uv_check_start(check(), Check::check_callback);
    UVP_LOG_ERROR(r);
    return r;
  }
};

class CheckObject : public Check {
  mutable uv::CheckT _check;

public:
  virtual uv::HandleT *handle() const override {
    return (uv::HandleT *)&_check;
  }

  virtual uv::CheckT *check() const override { return &_check; }

  CheckObject(Loop *loop) {
    int r = uv_check_init(loop->loop(), check());
    UVP_LOG_ERROR_EXIT(r);
    uv_handle_set_data(handle(), this);
  }
  virtual ~CheckObject() {}
};

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

  Idle() {}

public:
  virtual ~Idle() {}

  virtual uv::IdleT *idle() const = 0;

  int start(const IdleCallback &cb) {
    UVP_ASSERT(cb);
    _impl._idleCallback = cb;
    int r = uv_idle_start(idle(), idle_callback);
    UVP_LOG_ERROR(r);
    return r;
  }

  int stop() {
    int r = uv_idle_stop(idle());
    UVP_LOG_ERROR(r);
    return r;
  }

  // extend functions
  IdleCallback startCallback() const { return _impl._idleCallback; }
  void startCallback(const IdleCallback &cb) {
    UVP_ASSERT(cb);
    _impl._idleCallback = cb;
  }
  int start() {
    int r = uv_idle_start(idle(), idle_callback);
    UVP_LOG_ERROR(r);
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
    UVP_LOG_ERROR_EXIT(r);
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

  Timer() {}

public:
  virtual ~Timer() {}

  virtual uv::TimerT *timer() const = 0;

  int start(const TimerCallback &cb, UnsignedLong timeout,
            UnsignedLong repeat) {
    UVP_ASSERT(cb);
    _impl._timerCallback = cb;
    int r = uv_timer_start(timer(), timer_callback, timeout, repeat);
    UVP_LOG_ERROR(r);
    return r;
  }

  int stop() {
    int r = uv_timer_stop(timer());
    UVP_LOG_ERROR(r);
    return r;
  }

  int again() {
    int r = uv_timer_again(timer());
    UVP_LOG_ERROR(r);
    return r;
  }

  void repeat(uint64_t repeat) { uv_timer_set_repeat(timer(), repeat); }

  UnsignedLong repeat() const { return uv_timer_get_repeat(timer()); }

  // extend functions
  TimerCallback startCallback() const { return _impl._timerCallback; }
  void startCallback(const TimerCallback &cb) {
    UVP_ASSERT(cb);
    _impl._timerCallback = cb;
  }
  int start(UnsignedLong timeout, UnsignedLong repeat) {
    int r = uv_timer_start(timer(), timer_callback, timeout, repeat);
    UVP_LOG_ERROR(r);
    return r;
  }
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
    UVP_LOG_ERROR_EXIT(r);

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

  Stream() {}

public:
  virtual ~Stream() {}

  virtual uv::StreamT *stream() const = 0;

  int shutdown(const ShutdownCallback &cb) {
    auto req = allocAnyReq<uv_shutdown_t>(nullptr, 0);
    int r = uv_shutdown(&req->req, stream(), cb ? _impl._shutdownCallback = cb,
                        shutdown_callback : nullptr);
    if (r) {
      freeAnyReq(req);
      UVP_LOG_ERROR(r);
    }
    return r;
  }

  int listen(int backlog, const ConnectionCallback &cb) {
    UVP_ASSERT(cb);
    _impl._connectionCallback = cb;

    int r = uv_listen(stream(), backlog, connection_callback);
    UVP_LOG_ERROR(r);
    return r;
  }

  int accept(Stream *client) {
    int r = uv_accept(stream(), client->stream());
    UVP_LOG_ERROR(r);
    return r;
  }

  int readStart(const Handle::AllocCallback &alloc_cb,
                const ReadCallback &read_cb) {
    UVP_ASSERT(read_cb);
    _impl._readCallback = read_cb;
    Handle::_impl._allocCallback = alloc_cb;

    int r = uv_read_start(stream(), Handle::alloc_callback, read_callback);
    UVP_LOG_ERROR(r);
    return r;
  }

  int readStop() {
    int r = uv_read_stop(stream());
    UVP_LOG_ERROR(r);
    return r;
  }

  int write(const uv::BufT bufs[], unsigned int nbufs,
            const WriteCallback &cb) {
    auto req = allocAnyReq<uv::WriteT>(bufs, nbufs);
    int r = uv_write((uv::WriteT *)req, stream(), bufs, nbufs,
                     cb ? _impl._writeCallback = cb, write_callback : nullptr);
    if (r) {
      freeAnyReq(req);
      UVP_LOG_ERROR(r);
    }
    return r;
  }

  int write2(const uv::BufT bufs[], unsigned int nbufs, Stream *sendstream,
             const WriteCallback &cb) {
    auto req = allocAnyReq<uv::WriteT>(bufs, nbufs);
    int r = uv_write2((uv::WriteT *)req, stream(), bufs, nbufs,
                      sendstream->stream(), cb ? _impl._writeCallback = cb,
                      write_callback : nullptr);
    if (r) {
      freeAnyReq(req);
      UVP_LOG_ERROR(r);
    }
    return r;
  }

  int tryWrite(uv::BufT bufs[], unsigned int nbufs) {
    int r = uv_try_write(stream(), bufs, nbufs);
    UVP_LOG_ERROR(r);
    return r;
  }

  int isReadable() const { return uv_is_readable(stream()); }

  int isWritable() const { return uv_is_writable(stream()); }

  int setBlocking(int blocking) {
    int r = uv_stream_set_blocking(stream(), blocking);
    UVP_LOG_ERROR(r);
    return r;
  }

  int getWriteQueueSize() const {
    return uv_stream_get_write_queue_size(stream());
  }

  // extend functions
  void shutdownCallback(const ShutdownCallback &cb) {
    _impl._shutdownCallback = cb;
  }

  ShutdownCallback shutdownCallback() const { return _impl._shutdownCallback; }

  void connectionCallback(const ConnectionCallback &cb) {
    UVP_ASSERT(cb);
    _impl._connectionCallback = cb;
  }

  ConnectionCallback connectionCallback() const {
    return _impl._connectionCallback;
  }

  void readCallback(const ReadCallback &cb) {
    UVP_ASSERT(cb);
    _impl._readCallback = cb;
  }

  ReadCallback readCallback() const { return _impl._readCallback; }

  void writeCallback(const WriteCallback &cb) { _impl._writeCallback = cb; }

  WriteCallback writeCallback() const { return _impl._writeCallback; }

  int shutdown() {
    auto req = allocAnyReq<uv_shutdown_t>(nullptr, 0);
    int r = uv_shutdown(&req->req, stream(),
                        _impl._shutdownCallback ? shutdown_callback : nullptr);
    if (r) {
      freeAnyReq(req);
      UVP_LOG_ERROR(r);
    }
    return r;
  }

  int listen(int backlog) {
    UVP_ASSERT(_impl._connectionCallback);
    int r = uv_listen(stream(), backlog, connection_callback);
    UVP_LOG_ERROR(r);
    return r;
  }

  int readStart() {
    UVP_ASSERT(_impl._readCallback);
    int r = uv_read_start(stream(), Handle::alloc_callback, read_callback);
    UVP_LOG_ERROR(r);
    return r;
  }

  int write(const uv::BufT bufs[], unsigned int nbufs) {
    auto req = allocAnyReq<uv::WriteT>(bufs, nbufs);
    int r = uv_write((uv::WriteT *)req, stream(), bufs, nbufs,
                     _impl._writeCallback ? write_callback : nullptr);
    if (r) {
      freeAnyReq(req);
      UVP_LOG_ERROR(r);
    }
    return r;
  }

  int write2(const uv::BufT bufs[], unsigned int nbufs, Stream *sendstream) {
    auto req = allocAnyReq<uv::WriteT>(bufs, nbufs);
    int r = uv_write2((uv::WriteT *)req, stream(), bufs, nbufs,
                      sendstream->stream(),
                      _impl._writeCallback ? write_callback : nullptr);
    if (r) {
      freeAnyReq(req);
      UVP_LOG_ERROR(r);
    }
    return r;
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

  StreamObject() { uv_handle_set_data((uv::HandleT *)&_stream, this); }
  virtual ~StreamObject() {}
};

// --

class Pipe : public Stream {
protected:
  Pipe() {}

public:
  virtual ~Pipe() {}

  virtual uv::PipeT *pipe() const = 0;

  int open(uv::File file) {
    int r = uv_pipe_open(pipe(), file);
    UVP_LOG_ERROR(r);
    return r;
  }

  int bind(const char *name) {
    int r = uv_pipe_bind(pipe(), name);
    UVP_LOG_ERROR(r);
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
    UVP_LOG_ERROR(r);
    return r;
  }

  int getPeername(char *buffer, size_t *size) const {
    int r = uv_pipe_getpeername(pipe(), buffer, size);
    UVP_LOG_ERROR(r);
    return r;
  }

  void pendingInstances(int count) { uv_pipe_pending_instances(pipe(), count); }

  int pendingCount() {
    int r = uv_pipe_pending_count(pipe());
    UVP_LOG_ERROR(r);
    return r;
  }

  Handle::Type pendingType() { return uv_pipe_pending_type(pipe()); }

  int chmod(int flags) {
    int r = uv_pipe_chmod(pipe(), flags);
    UVP_LOG_ERROR(r);
    return r;
  }

  // extend functions
  void connectCallback(const ConnectCallback &cb) {
    UVP_ASSERT(cb);
    Stream::_impl._connectCallback = cb;
  }

  ConnectCallback connectCallback() const {
    return Stream::_impl._connectCallback;
  }
  
  void connect(const char *name) {
    UVP_ASSERT(Stream::_impl._connectCallback);
    auto req = allocAnyReq<uv::ConnectT>(nullptr, 0);
    uv_pipe_connect(&req->req, pipe(), name, Stream::connect_callback);
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
    UVP_LOG_ERROR_EXIT(r);

    uv_handle_set_data(handle(), this);
  }
  ~PipeObject() {}
};

// --

class Tcp : public Stream {
protected:
  Tcp() {}

public:
  virtual ~Tcp() {}

  virtual uv::TcpT *tcp() const = 0;

  int open(uv::OsSockT sock) {
    int r = uv_tcp_open(tcp(), sock);
    UVP_LOG_ERROR(r);
    return r;
  }

  int nodelay(int enable) {
    int r = uv_tcp_nodelay(tcp(), enable);
    UVP_LOG_ERROR(r);
    return r;
  }

  int keepalive(int enable, unsigned int delay) {
    int r = uv_tcp_keepalive(tcp(), enable, delay);
    UVP_LOG_ERROR(r);
    return r;
  }

  int simultaneousAcepts(int enable) {
    int r = uv_tcp_simultaneous_accepts(tcp(), enable);
    UVP_LOG_ERROR(r);
    return r;
  }

  int bind(const struct sockaddr *addr, unsigned int flags) {
    int r = uv_tcp_bind(tcp(), addr, flags);
    UVP_LOG_ERROR(r);
    return r;
  }

  int getsockname(struct sockaddr *name, int *namelen) const {
    int r = uv_tcp_getsockname(tcp(), name, namelen);
    UVP_LOG_ERROR(r);
    return r;
  }

  int getpeername(struct sockaddr *name, int *namelen) const {
    int r = uv_tcp_getpeername(tcp(), name, namelen);
    UVP_LOG_ERROR(r);
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
      UVP_LOG_ERROR(r);
    }
    return r;
  }

  // extend functions
  void connectCallback(const ConnectCallback &cb) {
    UVP_ASSERT(cb);
    Stream::_impl._connectCallback = cb;
  }

  ConnectCallback connectCallback() const {
    return Stream::_impl._connectCallback;
  }

  int connect(const struct sockaddr *addr) {
    UVP_ASSERT(Stream::_impl._connectCallback);
    auto req = allocAnyReq<uv::ConnectT>(nullptr, 0);
    int r = uv_tcp_connect(&req->req, tcp(), addr, Stream::connect_callback);
    if (r) {
      // 如果失败，释放申请的req。如果成功，在connect_cb中释放req。
      freeAnyReq(req);
      UVP_LOG_ERROR(r);
    }
    return r;
  }

  int reinit() {
    int r = uv_tcp_init(loop()->loop(), tcp());
    UVP_LOG_ERROR(r);
    uv_handle_set_data(handle(), this);
    return r;
  }

  std::string peer() const {
    sockaddr addr = {0};
    sockaddr_in *paddr_in = (sockaddr_in *)&addr;

    int len = sizeof(addr);
    int r = getpeername(&addr, &len);
    UVP_LOG_ERROR(r);

    char name[INET6_ADDRSTRLEN] = {0};
    r = inetNtop(paddr_in->sin_family, (const void *)&paddr_in->sin_addr, name,
                 sizeof(name));
    UVP_LOG_ERROR(r);
    int port = ntohs(paddr_in->sin_port);

    std::stringstream ss;
    ss << name << ":" << port;
    return ss.str();
  }

  std::string name() const {
    sockaddr addr = {0};
    sockaddr_in *paddr_in = (sockaddr_in *)&addr;

    int len = sizeof(addr);
    int r = getsockname(&addr, &len);
    UVP_LOG_ERROR(r);

    char name[INET6_ADDRSTRLEN] = {0};
    r = inetNtop(paddr_in->sin_family, (const void *)&paddr_in->sin_addr, name,
                 sizeof(name));
    UVP_LOG_ERROR(r);
    int port = ntohs(paddr_in->sin_port);

    std::stringstream ss;
    ss << name << ":" << port;
    return ss.str();
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
    UVP_LOG_ERROR_EXIT(r);

    uv_handle_set_data(handle(), this);
  }
  TcpObject(Loop *loop, unsigned int flags) {
    int r = uv_tcp_init_ex(loop->loop(), &_tcp, flags);
    UVP_LOG_ERROR_EXIT(r);

    uv_handle_set_data(handle(), this);
  }
  virtual ~TcpObject() {}
};

// --

class Tty : public Stream {
protected:
  Tty() {}

public:
  virtual ~Tty() {}

  using TtyMode = uv::TtyMode;

  virtual uv::TtyT *tty() const = 0;

  static int resetMode() {
    int r = uv_tty_reset_mode();
    UVP_LOG_ERROR(r);
    return r;
  }

  int setMode(TtyMode mode) {
    int r = uv_tty_set_mode(tty(), mode);
    UVP_LOG_ERROR(r);
    return r;
  }

  int getWinsize(int *width, int *height) {
    int r = uv_tty_get_winsize(tty(), width, height);
    UVP_LOG_ERROR(r);
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
    UVP_LOG_ERROR_EXIT(r);

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

  static void send_callback(uv::UdpSendT *req, int status) {
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
  }

  Udp() {}

public:
  virtual ~Udp() {}

  virtual uv::UdpT *udp() const = 0;

  int open(uv::OsSockT sock) {
    int r = uv_udp_open(udp(), sock);
    UVP_LOG_ERROR(r);
    return r;
  }

  int bind(const struct sockaddr *addr, unsigned int flags) {
    int r = uv_udp_bind(udp(), addr, flags);
    UVP_LOG_ERROR(r);
    return r;
  }

  int getsockname(struct sockaddr *name, int *namelen) const {
    int r = uv_udp_getsockname(udp(), name, namelen);
    UVP_LOG_ERROR(r);
    return r;
  }

  int setMembership(const char *multicast_addr, const char *interface_addr,
                    MemberShip membership) {
    int r = uv_udp_set_membership(udp(), multicast_addr, interface_addr,
                                  membership);
    UVP_LOG_ERROR(r);
    return r;
  }

  int setMulticastLoop(int on) {
    int r = uv_udp_set_multicast_loop(udp(), on);
    UVP_LOG_ERROR(r);
    return r;
  }
  int setMulticastTtl(int ttl) {
    int r = uv_udp_set_multicast_ttl(udp(), ttl);
    UVP_LOG_ERROR(r);
    return r;
  }

  int setMulticastInterface(const char *interface_addr) {
    int r = uv_udp_set_multicast_interface(udp(), interface_addr);
    UVP_LOG_ERROR(r);
    return r;
  }

  int setBroadcast(int on) {
    int r = uv_udp_set_broadcast(udp(), on);
    UVP_LOG_ERROR(r);
    return r;
  }

  int setTtl(int ttl) {
    int r = uv_udp_set_ttl(udp(), ttl);
    UVP_LOG_ERROR(r);
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
      UVP_LOG_ERROR(r);
    }
    return r;
  }

  int trySend(const uv::BufT bufs[], unsigned int nbufs,
              const struct sockaddr *addr) {
    int r = uv_udp_try_send(udp(), bufs, nbufs, addr);
    UVP_LOG_ERROR(r);
    return r;
  }

  int recvStart(Stream::AllocCallback alloc_cb, RecvCallback recv_cb) {
    UVP_ASSERT(recv_cb);
    _impl._recvCallback = recv_cb;
    Handle::_impl._allocCallback = alloc_cb;

    int r = uv_udp_recv_start(udp(), Handle::alloc_callback, recv_callback);
    UVP_LOG_ERROR(r);
    return r;
  }

  int recvStop() {
    int r = uv_udp_recv_stop(udp());
    UVP_LOG_ERROR(r);
    return r;
  }

  int getSendQueueSize() const {
    int r = uv_udp_get_send_queue_size(udp());
    UVP_LOG_ERROR(r);
    return r;
  }

  int getSendQueueCount() const {
    int r = uv_udp_get_send_queue_count(udp());
    UVP_LOG_ERROR(r);
    return r;
  }

  // extend functions
  void sendCallback(const SendCallback &cb) {
    UVP_ASSERT(cb);
    _impl._sendCallback = cb;
  }

  SendCallback sendCallback() const { return _impl._sendCallback; }

  void recvCallback(const RecvCallback &cb) {
    UVP_ASSERT(cb);
    _impl._recvCallback = cb;
  }

  RecvCallback recvCallback() const { return _impl._recvCallback; }

  int recvStart() {
    UVP_ASSERT(_impl._recvCallback);
    int r = uv_udp_recv_start(udp(), Handle::alloc_callback, recv_callback);
    UVP_LOG_ERROR(r);
    return r;
  }

  int send(const uv::BufT bufs[], unsigned int nbufs,
           const struct sockaddr *addr) {
    UVP_ASSERT(_impl._sendCallback);
    auto req = allocAnyReq<uv::UdpSendT>(bufs, nbufs);
    int r = uv_udp_send((uv::UdpSendT *)req, udp(), bufs, nbufs, addr,
                        send_callback);
    if (r) {
      freeAnyReq(req);
      UVP_LOG_ERROR(r);
    }
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
    UVP_LOG_ERROR_EXIT(r);

    uv_handle_set_data(handle(), this);
  }
  UdpObject(Loop *loop, unsigned int flags) {
    int r = uv_udp_init_ex(loop->loop(), &_udp, flags);
    UVP_LOG_ERROR_EXIT(r);

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

  Async() {}

public:
  virtual ~Async() {}

  virtual uv::AsyncT *async() const = 0;

  int send() {
    int r = uv_async_send(async());
    UVP_LOG_ERROR(r);
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
    UVP_LOG_ERROR_EXIT(r);

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

  Signal() {}

public:
  virtual ~Signal() {}

  virtual uv::SignalT *signal() const = 0;

  int start(const SignalCallback &cb, int signum) {
    UVP_ASSERT(cb);
    _impl._signalCallback = cb;

    int r = uv_signal_start(signal(), signal_callback, signum);
    UVP_LOG_ERROR(r);
    return r;
  }

  int startOneshort(const SignalCallback &cb, int signum) {
    UVP_ASSERT(cb);
    _impl._signalCallback = cb;

    int r = uv_signal_start_oneshot(signal(), signal_callback, signum);
    UVP_LOG_ERROR(r);
    return r;
  }

  int stop() {
    int r = uv_signal_stop(signal());
    UVP_LOG_ERROR(r);
    return r;
  }

  // extend functions
  void startCallback(const SignalCallback &cb) {
    UVP_ASSERT(cb);
    _impl._signalCallback = cb;
  }

  SignalCallback startCallback() const { return _impl._signalCallback; }

  int start(int signum) {
    UVP_ASSERT(_impl._signalCallback);
    int r = uv_signal_start(signal(), signal_callback, signum);
    UVP_LOG_ERROR(r);
    return r;
  }

  int startOneshort(int signum) {
    UVP_ASSERT(_impl._signalCallback);
    int r = uv_signal_start_oneshot(signal(), signal_callback, signum);
    UVP_LOG_ERROR(r);
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
    UVP_LOG_ERROR_EXIT(r);

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
    Callback exit_cb;
    const char *file;
    char **args;
    char **env;
    const char *cwd;
    unsigned int flags;
    int stdio_count;
    StdioContainer *stdio;
    Uid uid;
    Gid gid;
  };

protected:
  struct Impl {
    Callback _callback;
  };
  Impl _impl;

  static void callback(uv::ProcessT *handle, int64_t exit_status,
                       int term_signal) {
    auto p = (Process *)uv_handle_get_data((uv::HandleT *)handle);
    if (p->_impl._callback) {
      p->_impl._callback(p, exit_status, term_signal);
    } else {
      UVP_ASSERT(false);
    }
  }

  static void disableStdioInheritance() { uv_disable_stdio_inheritance(); }

  static int kill(int pid, int signum) {
    int r = uv_kill(pid, signum);
    UVP_LOG_ERROR(r);
    return r;
  }

  Process() {}

public:
  virtual ~Process() {}

  virtual uv::ProcessT *process() const = 0;

  int spawn(Loop *loop, const Options *options) {
    _impl._callback = options->exit_cb;

    uv::ProcessOptionsT opt = {0};
    opt.args = options->args;
    opt.cwd = options->cwd;
    opt.env = options->env;
    opt.exit_cb = options->exit_cb ? Process::callback : nullptr;
    opt.file = options->file;
    opt.flags = options->flags;
    opt.gid = options->gid;
    opt.stdio = options->stdio;
    opt.stdio_count = options->stdio_count;
    opt.uid = options->uid;

    int r = uv_spawn(loop->loop(), process(), &opt);
    UVP_LOG_ERROR(r);
    return r;
  }

  int kill(int signum) {
    int r = uv_process_kill(process(), signum);
    UVP_LOG_ERROR(r);
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
  using StartCallback = std::function<void(FsEvent *, const char *, int, int)>;

protected:
  struct Impl {
    StartCallback _startCallback;
  };
  Impl _impl;

  static void start_callback(uv::FsEventT *handle, const char *filename,
                             int events, int status) {
    auto p = (FsEvent *)uv_handle_get_data((uv::HandleT *)handle);
    if (p->_impl._startCallback) {
      p->_impl._startCallback(p, filename, events, status);
    }
  }

  FsEvent() {}

public:
  virtual ~FsEvent() {}
  
  virtual uv::FsEventT *fsEvent() const = 0;

  int start(const StartCallback &cb, const char *path, unsigned int flags) {
    UVP_ASSERT(cb);
    _impl._startCallback = cb;

    int r = uv_fs_event_start(fsEvent(), FsEvent::start_callback, path, flags);
    UVP_LOG_ERROR(r);
    return r;
  }

  int stop() {
    int r = uv_fs_event_stop(fsEvent());
    UVP_LOG_ERROR(r);
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
    UVP_LOG_ERROR(r);
    return r;
  }

  // extend functions
  void startCallback(const StartCallback &cb) {
    UVP_ASSERT(cb);
    _impl._startCallback = cb;
  }

  StartCallback startCallback() const { return _impl._startCallback; }
  
  int start(const char *path, unsigned int flags) {
    UVP_ASSERT(_impl._startCallback);
    int r = uv_fs_event_start(fsEvent(), FsEvent::start_callback, path, flags);
    UVP_LOG_ERROR(r);
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
    UVP_LOG_ERROR_EXIT(r);

    uv_handle_set_data(handle(), this);
  }
  virtual ~FsEventObject() {}
};

} // namespace uvp
