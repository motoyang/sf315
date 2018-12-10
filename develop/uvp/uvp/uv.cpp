#include <queue>
#include <list>

#include <utilites.hpp>
#include <misc.hpp>
#include <uv.hpp>

// --

// --

template <typename T> struct AnyReq {
  T req;
  uv_buf_t buf;
};

template <typename T>
AnyReq<T> *allocAnyReq(const uv_buf_t bufs[], size_t nbufs) {
  auto req = (AnyReq<T> *)malloc(sizeof(AnyReq<T>));
  if (!req) {
    LOG_IF_ERROR_EXIT(UV_ENOMEM);
  }

  size_t len = sizeof(uv_buf_t) * nbufs;
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
  if (req->buf.base) {
    free(req->buf.base);
  }
  free(req);
}

// --

// --

struct LoopT::Impl {
  Impl() : _loop(std::make_unique<uv_loop_t>()), _owner(true) {}
  Impl(uv_loop_t *p) : _loop(p), _owner(false) {}
  virtual ~Impl() {
    if (!_owner) {
      _loop.release();
    }
  }
  std::unique_ptr<uv_loop_t> _loop;
  bool _owner;
  void *_data;
  WalkCallback _walkCallback;
  static void walk_callback(uv_handle_t *handle, void *arg);
};

void LoopT::Impl::walk_callback(uv_handle_t *handle, void *arg) {
  auto h = (HandleI *)uv_handle_get_data(handle);
  auto p = (LoopT *)uv_loop_get_data(uv_handle_get_loop(handle));
  p->_impl->_walkCallback(h, arg);
}

// --

std::unique_ptr<LoopT> LoopT::defaultLoop() {
  uv_loop_t *loop = uv_default_loop();
  if (!loop) {
    LOG_IF_ERROR_EXIT(UV_ENOMEM);
  }
  return std::unique_ptr<LoopT>(new LoopT(loop));
}

size_t LoopT::size() { return uv_loop_size(); }

LoopT::LoopT() : _impl(std::make_unique<LoopT::Impl>()) {
  int r = uv_loop_init(_impl->_loop.get());
  LOG_IF_ERROR_EXIT(r);

  uv_loop_set_data(_impl->_loop.get(), this);
}

LoopT::LoopT(uv_loop_t *l) : _impl(std::make_unique<LoopT::Impl>(l)) {
  uv_loop_set_data(_impl->_loop.get(), this);
}

LoopT::~LoopT() {}

int LoopT::configure(std::initializer_list<LoopOperation> options) {
  int r = 0;
  for (auto ptr = options.begin(); ptr != options.end(); ptr++) {
    r = uv_loop_configure(_impl->_loop.get(), *ptr);
    LOG_IF_ERROR_RETURN(r);
  }
  return r;
}

int LoopT::close() {
  int r = uv_loop_close(_impl->_loop.get());
  LOG_IF_ERROR(r);
  return r;
}

int LoopT::run(LoopT::RunMode mode) {
  int r = uv_run(_impl->_loop.get(), mode);
  LOG_IF_ERROR(r);
  return r;
}

bool LoopT::alive() const { return uv_loop_alive(_impl->_loop.get()); }

void LoopT::stop() { uv_stop(_impl->_loop.get()); }

int LoopT::backendFd() const { return uv_backend_fd(_impl->_loop.get()); }

int LoopT::backendTimeout() const {
  return uv_backend_timeout(_impl->_loop.get());
}

uint64_t LoopT::now() const { return uv_now(get()); }

void LoopT::updateTime() { return uv_update_time(_impl->_loop.get()); }

void LoopT::walk(WalkCallback &&cb, void *arg) {
  _impl->_walkCallback = std::forward<WalkCallback>(cb);
  uv_walk(_impl->_loop.get(), LoopT::Impl::walk_callback, arg);
}

int LoopT::fork() {
  int r = uv_loop_fork(_impl->_loop.get());
  LOG_IF_ERROR(r);
  return r;
}

void *LoopT::data() const { return _impl->_data; }

void *LoopT::data(void *data) {
  _impl->_data = data;
  return data;
}

uv_loop_t *LoopT::get() const { return _impl->_loop.get(); }

// --

struct HandleI::Impl {
  virtual ~Impl();

  void *_data = 0;
  size_t _bufSize = 128;
  size_t _queueSize = 2;
  std::queue<BufT> _buffers;

  AllocCallback _allocCallback;
  FreeCallback _freeCallback;
  CloseCallback _closeCallback;

  void defualtAllocCallback(size_t len, BufT *buf);
  void defaultFreeCallback(BufT buf);
  void defaultCloseCallback();

  static void alloc_callback(uv_handle_t *handle, size_t suggested_size,
                             BufT *buf);
  static void close_callback(uv_handle_t *handle);
};

HandleI::Impl::~Impl() {
  while (_buffers.size()) {
    auto b = _buffers.front();
    _buffers.pop();
    freeBuf(b);
  }
}

void HandleI::Impl::defualtAllocCallback(size_t len, BufT *buf) {
  if (_buffers.size() > 0) {
    *buf = _buffers.front();
    _buffers.pop();
  } else {
    if (_bufSize) {
      len = _bufSize;
    }
    *buf = allocBuf(len);
  }
}

void HandleI::Impl::defaultFreeCallback(BufT buf) {
  if (_buffers.size() < _queueSize) {
    _buffers.push(buf);
  } else {
    freeBuf(buf);
  }
}

void HandleI::Impl::defaultCloseCallback() {}

void HandleI::Impl::alloc_callback(uv_handle_t *handle, size_t suggested_size,
                                   BufT *buf) {
  auto h = (HandleI *)uv_handle_get_data(handle);
  h->_impl->_allocCallback(suggested_size, buf);
}

void HandleI::Impl::close_callback(uv_handle_t *handle) {
  auto h = (HandleI *)uv_handle_get_data(handle);
  h->_impl->_closeCallback();
}

// --

size_t HandleI::size(HandleI::Type type) { return uv_handle_size(type); }

const char *HandleI::typeName(HandleI::Type type) {
  return uv_handle_type_name(type);
}

HandleI::HandleI() : _impl(std::make_unique<HandleI::Impl>()) {
  _impl->_allocCallback =
      std::bind(&HandleI::Impl::defualtAllocCallback, _impl.get(),
                std::placeholders::_1, std::placeholders::_2);
  _impl->_freeCallback = std::bind(&HandleI::Impl::defaultFreeCallback,
                                   _impl.get(), std::placeholders::_1);
  _impl->_closeCallback =
      std::bind(&HandleI::Impl::defaultCloseCallback, _impl.get());
}

HandleI::~HandleI() {}

bool HandleI::isActive() const { return uv_is_active(getHandle()); }

bool HandleI::isClosing() const { return uv_is_closing(getHandle()); }

void HandleI::ref() { uv_ref(getHandle()); }

void HandleI::unref() { uv_unref(getHandle()); }

bool HandleI::hasRef() const { return uv_has_ref(getHandle()); }

int HandleI::sendBufferSize(int *value) {
  return uv_send_buffer_size(getHandle(), value);
}

int HandleI::recvBufferSize(int *value) {
  return uv_recv_buffer_size(getHandle(), value);
}

int HandleI::fileno(OsFdT *fd) { return uv_fileno(getHandle(), fd); }

LoopT *HandleI::loop() const {
  uv_loop_t *loop = uv_handle_get_loop(getHandle());
  return (LoopT *)uv_loop_get_data(loop);
}

void *HandleI::data() const { return _impl->_data; }

void *HandleI::data(void *data) { return _impl->_data = data; }

HandleI::Type HandleI::type() const { return uv_handle_get_type(getHandle()); }

HandleI::AllocCallback HandleI::allocCallback() const {
  return _impl->_allocCallback;
}

void HandleI::allocCallback(const HandleI::AllocCallback &cb) {
  _impl->_allocCallback = cb;
}

HandleI::FreeCallback HandleI::freeCallback() const {
  return _impl->_freeCallback;
}

void HandleI::freeCallback(const HandleI::FreeCallback &cb) {
  _impl->_freeCallback = cb;
}

HandleI::CloseCallback HandleI::closeCallback() const {
  return _impl->_closeCallback;
}

void HandleI::closeCallback(const HandleI::CloseCallback &cb) {
  _impl->_closeCallback = cb;
}

void HandleI::close() { uv_close(getHandle(), HandleI::Impl::close_callback); }

void HandleI::setDefaultSize(size_t bufSize, size_t queueSize) {
  _impl->_bufSize = bufSize;
  _impl->_queueSize = queueSize;
}

// --

uv_handle_t *HandleT::getHandle() const { return (uv_handle_t *)&_handle; }

HandleT::HandleT() { uv_handle_set_data(getHandle(), this); }

HandleT::~HandleT() {}

// --

struct IdleI::Impl {
  IdleCallback _idleCallback;
  static void idle_callback(uv_idle_t *idle);
};

void IdleI::Impl::idle_callback(uv_idle_t *idle) {
  auto p = (IdleI *)uv_handle_get_data((uv_handle_t *)idle);
  p->_impl->_idleCallback();
}

// --

IdleI::IdleI() : _impl(std::make_unique<IdleI::Impl>()) {}

IdleI::~IdleI() {}

int IdleI::start() {
  int r = uv_idle_start(getIdle(), IdleI::Impl::idle_callback);
  LOG_IF_ERROR(r);
  return r;
}

int IdleI::stop() {
  int r = uv_idle_stop(getIdle());
  LOG_IF_ERROR(r);
  return r;
}

IdleI::IdleCallback IdleI::idleCallback() const { return _impl->_idleCallback; }

void IdleI::idleCallback(const IdleI::IdleCallback &cb) {
  _impl->_idleCallback = cb;
}

// --

uv_handle_t *IdleT::getHandle() const { return (uv_handle_t *)&_idle; }

uv_idle_t *IdleT::getIdle() const { return (uv_idle_t *)&_idle; }

IdleT::IdleT(LoopT *loop) {
  int r = uv_idle_init(loop->get(), &_idle);
  LOG_IF_ERROR_EXIT(r);

  uv_handle_set_data(getHandle(), this);
}

IdleT::~IdleT() {}

// --

struct TimerI::Impl {
  TimerCallback _timerCallback;
  static void timer_callback(uv_timer_t *handle);
};

void TimerI::Impl::timer_callback(uv_timer_t *handle) {
  auto p = (TimerI *)uv_handle_get_data((uv_handle_t *)handle);
  if (p->_impl->_timerCallback) {
    p->_impl->_timerCallback();
  }
}

// --

TimerI::TimerI() : _impl(std::make_unique<TimerI::Impl>()) {}

TimerI::~TimerI() {}

int TimerI::start(uint64_t timeout, uint64_t repeat) {
  int r =
      uv_timer_start(getTimer(), TimerI::Impl::timer_callback, timeout, repeat);
  LOG_IF_ERROR(r);
  return r;
}

int TimerI::stop() {
  int r = uv_timer_stop(getTimer());
  LOG_IF_ERROR(r);
  return r;
}

int TimerI::again() {
  int r = uv_timer_again(getTimer());
  LOG_IF_ERROR(r);
  return r;
}

void TimerI::repeat(uint64_t repeat) {
  uv_timer_set_repeat(getTimer(), repeat);
}

uint64_t TimerI::repeat() const { return uv_timer_get_repeat(getTimer()); }

void TimerI::timerCallback(const TimerI::TimerCallback &cb) {
  _impl->_timerCallback = cb;
}

TimerI::TimerCallback TimerI::timerCallback() const {
  return _impl->_timerCallback;
}

// --

uv_handle_t *TimerT::getHandle() const { return (uv_handle_t *)&_timer; }

uv_timer_t *TimerT::getTimer() const { return (uv_timer_t *)&_timer; }

TimerT::TimerT(LoopT *loop) {
  int r = uv_timer_init(loop->get(), &_timer);
  LOG_IF_ERROR_EXIT(r);

  uv_handle_set_data(getHandle(), this);
}

TimerT::~TimerT() {}

// --

struct StreamI::Impl {
  Impl();

  ConnectCallback _connectCallback;
  ReadCallback _readCallback;
  WriteCallback _writeCallback;
  ShutdownCallback _shutdownCallback;
  ConnectionCallback _connectionCallback;

  static void connect_callback(uv_connect_t *req, int status);
  static void read_callback(uv_stream_t *stream, ssize_t nread,
                            const BufT *buf);
  static void write_callback(uv_write_t *req, int status);
  static void shutdown_callback(uv_shutdown_t *req, int status);
  static void connection_callback(uv_stream_t *server, int status);
};

StreamI::Impl::Impl() {}

void StreamI::Impl::connect_callback(uv_connect_t *req, int status) {
  uv_stream_t *h = req->handle;
  auto p = (StreamI *)uv_handle_get_data((uv_handle_t *)h);

  if (p->_impl->_connectCallback) {
    p->_impl->_connectCallback(status);
  }

  // 释放在connect()中申请的req
  freeAnyReq((AnyReq<uv_connect_t> *)req);
}

void StreamI::Impl::read_callback(uv_stream_t *stream, ssize_t nread,
                                  const BufT *buf) {
  auto p = (StreamI *)uv_handle_get_data((uv_handle_t *)stream);
  if (p->_impl->_readCallback) {
    p->_impl->_readCallback(nread, buf);
  }

  // 释放在read时分配的memory，这个memory是通过Handle的allocCallback分配的。
  p->HandleI::_impl->_freeCallback(*buf);
}

void StreamI::Impl::write_callback(uv_write_t *req, int status) {
  uv_stream_t *h = req->handle;
  auto p = (StreamI *)uv_handle_get_data((uv_handle_t *)h);

  auto anyReq = (AnyReq<uv_write_t> *)req;
  // 在回调函数中，删除buf.base是用户的责任。
  if (p->_impl->_writeCallback) {
    p->_impl->_writeCallback(status, (uv_buf_t *)anyReq->buf.base,
                             anyReq->buf.len);
  }

  // 删除在write函数中申请的memory。
  freeAnyReq(anyReq);
}

void StreamI::Impl::shutdown_callback(uv_shutdown_t *req, int status) {
  uv_stream_t *h = req->handle;
  auto p = (StreamI *)uv_handle_get_data((uv_handle_t *)h);

  if (p->_impl->_shutdownCallback) {
    p->_impl->_shutdownCallback(status);
  }

  // 释放在shutdown()中申请的req
  freeAnyReq((AnyReq<uv_shutdown_t> *)req);
}

void StreamI::Impl::connection_callback(uv_stream_t *server, int status) {
  auto p = (StreamI *)uv_handle_get_data((uv_handle_t *)server);
  if (p->_impl->_connectionCallback) {
    p->_impl->_connectionCallback(status);
  }
}

StreamI::StreamI() : _impl(std::make_unique<StreamI::Impl>()) {}

StreamI::~StreamI() {}

int StreamI::accept(StreamI *client) {
  int r = uv_accept(getStream(), client->getStream());
  LOG_IF_ERROR(r);
  return r;
}

int StreamI::readStop() {
  int r = uv_read_stop(getStream());
  LOG_IF_ERROR(r);
  return r;
}

int StreamI::tryWrite(BufT bufs[], unsigned int nbufs) {
  int r = uv_try_write(getStream(), bufs, nbufs);
  LOG_IF_ERROR(r);
  return r;
}

int StreamI::isReadable() const { return uv_is_readable(getStream()); }

int StreamI::isWritable() const { return uv_is_writable(getStream()); }

int StreamI::setBlocking(int blocking) {
  int r = uv_stream_set_blocking(getStream(), blocking);
  LOG_IF_ERROR(r);
  return r;
}

int StreamI::getWriteQueueSize() const {
  return uv_stream_get_write_queue_size(getStream());
}

void StreamI::shutdownCallback(const StreamI::ShutdownCallback &cb) {
  _impl->_shutdownCallback = cb;
}

StreamI::ShutdownCallback StreamI::shutdownCallback() const {
  return _impl->_shutdownCallback;
}

void StreamI::connectionCallback(const StreamI::ConnectionCallback &cb) {
  _impl->_connectionCallback = cb;
}

StreamI::ConnectionCallback StreamI::connectionCallback() const {
  return _impl->_connectionCallback;
}

void StreamI::readCallback(const StreamI::ReadCallback &cb) {
  _impl->_readCallback = cb;
}

StreamI::ReadCallback StreamI::readCallback() const {
  return _impl->_readCallback;
}

void StreamI::writeCallback(const StreamI::WriteCallback &cb) {
  _impl->_writeCallback = cb;
}

StreamI::WriteCallback StreamI::writeCallback() const {
  return _impl->_writeCallback;
}

int StreamI::shutdown() {
  auto req = allocAnyReq<uv_shutdown_t>(nullptr, 0);
  int r = uv_shutdown(&req->req, getStream(), StreamI::Impl::shutdown_callback);
  if (r) {
    freeAnyReq(req);
    LOG_IF_ERROR(r);
  }
  return r;
}

int StreamI::listen(int backlog) {
  int r = uv_listen(getStream(), backlog, StreamI::Impl::connection_callback);
  LOG_IF_ERROR(r);
  return r;
}

int StreamI::readStart() {
  int r = uv_read_start(getStream(), HandleI::Impl::alloc_callback,
                        StreamI::Impl::read_callback);
  LOG_IF_ERROR(r);
  return r;
}

int StreamI::write(const BufT bufs[], unsigned int nbufs) {
  auto req = allocAnyReq<uv_write_t>(bufs, nbufs);
  int r = uv_write((uv_write_t *)req, getStream(), bufs, nbufs,
                   StreamI::Impl::write_callback);
  if (r) {
    freeAnyReq(req);
    LOG_IF_ERROR(r);
  }
  return r;
}

int StreamI::write2(const BufT bufs[], unsigned int nbufs,
                    StreamI *sendstream) {
  auto req = allocAnyReq<uv_write_t>(bufs, nbufs);
  int r = uv_write2((uv_write_t *)req, getStream(), bufs, nbufs,
                    sendstream->getStream(), StreamI::Impl::write_callback);
  if (r) {
    freeAnyReq(req);
    LOG_IF_ERROR(r);
  }
  return r;
}

// --

uv_handle_t *StreamT::getHandle() const { return (uv_handle_t *)&_stream; }

uv_stream_t *StreamT::getStream() const { return (uv_stream_t *)&_stream; }

StreamT::StreamT() { uv_handle_set_data((uv_handle_t *)&_stream, this); }

StreamT::~StreamT() {}

// --

int PipeI::open(File file) {
  int r = uv_pipe_open(getPipe(), file);
  LOG_IF_ERROR(r);
  return r;
}

int PipeI::bind(const char *name) {
  int r = uv_pipe_bind(getPipe(), name);
  LOG_IF_ERROR(r);
  return r;
}

void PipeI::connect(const char *name) {
  // uv_connect_t *req = StreamI::_impl->_connectRequestManager.alloc();
  auto req = allocAnyReq<uv_connect_t>(nullptr, 0);
  uv_pipe_connect(&req->req, getPipe(), name, StreamI::Impl::connect_callback);
}

int PipeI::getSockname(char *buffer, size_t *size) const {
  int r = uv_pipe_getsockname(getPipe(), buffer, size);
  LOG_IF_ERROR(r);
  return r;
}

int PipeI::getPeername(char *buffer, size_t *size) const {
  int r = uv_pipe_getpeername(getPipe(), buffer, size);
  LOG_IF_ERROR(r);
  return r;
}

void PipeI::pendingInstances(int count) {
  uv_pipe_pending_instances(getPipe(), count);
}

int PipeI::pendingCount() {
  int r = uv_pipe_pending_count(getPipe());
  LOG_IF_ERROR(r);
  return r;
}

HandleI::Type PipeI::pendingType() { return uv_pipe_pending_type(getPipe()); }

int PipeI::chmod(int flags) {
  int r = uv_pipe_chmod(getPipe(), flags);
  LOG_IF_ERROR(r);
  return r;
}

void PipeI::connectCallback(const StreamI::ConnectCallback &cb) {
  StreamI::_impl->_connectCallback = cb;
}

StreamI::ConnectCallback PipeI::connectCallback() const {
  return StreamI::_impl->_connectCallback;
}

// --

uv_handle_t *PipeT::getHandle() const { return (uv_handle_t *)&_pipe; }

uv_stream_t *PipeT::getStream() const { return (uv_stream_t *)&_pipe; }

uv_pipe_t *PipeT::getPipe() const { return (uv_pipe_t *)&_pipe; }

PipeT::PipeT(LoopT *loop, int ipc) {
  int r = uv_pipe_init(loop->get(), &_pipe, ipc);
  LOG_IF_ERROR_EXIT(r);

  uv_handle_set_data(getHandle(), this);
}

PipeT::~PipeT() {}

// --

int TcpI::open(OsSock sock) {
  int r = uv_tcp_open(getTcp(), sock);
  LOG_IF_ERROR(r);
  return r;
}

int TcpI::nodelay(int enable) {
  int r = uv_tcp_nodelay(getTcp(), enable);
  LOG_IF_ERROR(r);
  return r;
}

int TcpI::keepalive(int enable, unsigned int delay) {
  int r = uv_tcp_keepalive(getTcp(), enable, delay);
  LOG_IF_ERROR(r);
  return r;
}

int TcpI::simultaneousAcepts(int enable) {
  int r = uv_tcp_simultaneous_accepts(getTcp(), enable);
  LOG_IF_ERROR(r);
  return r;
}

int TcpI::bind(const struct sockaddr *addr, unsigned int flags) {
  int r = uv_tcp_bind(getTcp(), addr, flags);
  LOG_IF_ERROR(r);
  return r;
}

int TcpI::getsockname(struct sockaddr *name, int *namelen) {
  int r = uv_tcp_getsockname(getTcp(), name, namelen);
  LOG_IF_ERROR(r);
  return r;
}

int TcpI::getpeername(struct sockaddr *name, int *namelen) {
  int r = uv_tcp_getpeername(getTcp(), name, namelen);
  LOG_IF_ERROR(r);
  return r;
}

int TcpI::connect(const struct sockaddr *addr) {
  auto req = allocAnyReq<uv_connect_t>(nullptr, 0);
  int r = uv_tcp_connect(&req->req, getTcp(), addr,
                         StreamI::Impl::connect_callback);
  if (r) {
    // 如果失败，释放申请的req。如果成功，在connect_cb中释放req。
    freeAnyReq(req);
    LOG_IF_ERROR(r);
  }
  return r;
}

void TcpI::connectCallback(const StreamI::ConnectCallback &cb) {
  StreamI::_impl->_connectCallback = cb;
}

StreamI::ConnectCallback TcpI::connectCallback() const {
  return StreamI::_impl->_connectCallback;
}

// --

uv_handle_t *TcpT::getHandle() const { return (uv_handle_t *)&_tcp; }

uv_stream_t *TcpT::getStream() const { return (uv_stream_t *)&_tcp; }

uv_tcp_t *TcpT::getTcp() const { return (uv_tcp_t *)&_tcp; }

TcpT::TcpT(LoopT *loop) {
  int r = uv_tcp_init(loop->get(), &_tcp);
  LOG_IF_ERROR_EXIT(r);

  uv_handle_set_data(getHandle(), this);
}

TcpT::TcpT(LoopT *loop, unsigned int flags) {
  int r = uv_tcp_init_ex(loop->get(), &_tcp, flags);
  LOG_IF_ERROR_EXIT(r);

  uv_handle_set_data(getHandle(), this);
}

TcpT::~TcpT() {}

// --

int TtyI::resetMode() {
  int r = uv_tty_reset_mode();
  LOG_IF_ERROR(r);
  return r;
}

int TtyI::setMode(TtyI::TtyMode mode) {
  int r = uv_tty_set_mode(getTty(), mode);
  LOG_IF_ERROR(r);
  return r;
}

int TtyI::getWinsize(int *width, int *height) {
  int r = uv_tty_get_winsize(getTty(), width, height);
  LOG_IF_ERROR(r);
  return r;
}

// --

uv_handle_t *TtyT::getHandle() const { return (uv_handle_t *)&_tty; }

uv_stream_t *TtyT::getStream() const { return (uv_stream_t *)&_tty; }

uv_tty_t *TtyT::getTty() const { return (uv_tty_t *)&_tty; }

TtyT::TtyT(LoopT *loop, File fd, int unused) {
  int r = uv_tty_init(loop->get(), &_tty, fd, unused);
  LOG_IF_ERROR_EXIT(r);

  uv_handle_set_data(getHandle(), this);
}

TtyT::~TtyT() {}

// --

struct UdpI::Impl {
  Impl();

  SendCallback _sendCallback;
  RecvCallback _recvCallback;

  static void send_callback(uv_udp_send_t *req, int status);
  static void recv_callback(uv_udp_t *handle, ssize_t nread,
                            const uv_buf_t *buf, const struct sockaddr *addr,
                            unsigned flags);
};

UdpI::Impl::Impl() {}

void UdpI::Impl::send_callback(uv_udp_send_t *req, int status) {
  uv_udp_t *h = req->handle;
  auto p = (UdpI *)uv_handle_get_data((uv_handle_t *)h);

  auto anyReq = (AnyReq<uv_udp_send_t> *)req;
  if (p->_impl->_sendCallback) {
    p->_impl->_sendCallback(status, (uv_buf_t *)anyReq->buf.base,
                            anyReq->buf.len);
  }

  // 删除在send函数中申请的memory
  freeAnyReq(anyReq);
}

void UdpI::Impl::recv_callback(uv_udp_t *handle, ssize_t nread,
                               const uv_buf_t *buf, const struct sockaddr *addr,
                               unsigned flags) {
  auto p = (UdpI *)uv_handle_get_data((uv_handle_t *)handle);
  if (p->_impl->_recvCallback) {
    p->_impl->_recvCallback(nread, buf, addr, flags);
  }

  // 释放在recv时分配的memory，这个memory是通过Handle的allocCallback分配的。
  p->HandleI::_impl->_freeCallback(*buf);
}

// --

UdpI::UdpI() : _impl(std::make_unique<UdpI::Impl>()) {}

UdpI::~UdpI() {}

int UdpI::open(OsSock sock) {
  int r = uv_udp_open(getUdp(), sock);
  LOG_IF_ERROR(r);
  return r;
}

int UdpI::bind(const struct sockaddr *addr, unsigned int flags) {
  int r = uv_udp_bind(getUdp(), addr, flags);
  LOG_IF_ERROR(r);
  return r;
}

int UdpI::getsockname(struct sockaddr *name, int *namelen) const {
  int r = uv_udp_getsockname(getUdp(), name, namelen);
  LOG_IF_ERROR(r);
  return r;
}

int UdpI::setMembership(const char *multicast_addr, const char *interface_addr,
                        MemberShip membership) {
  int r = uv_udp_set_membership(getUdp(), multicast_addr, interface_addr,
                                membership);
  LOG_IF_ERROR(r);
  return r;
}

int UdpI::setMulticastLoop(int on) {
  int r = uv_udp_set_multicast_loop(getUdp(), on);
  LOG_IF_ERROR(r);
  return r;
}

int UdpI::setMulticastTtl(int ttl) {
  int r = uv_udp_set_multicast_ttl(getUdp(), ttl);
  LOG_IF_ERROR(r);
  return r;
}

int UdpI::setMulticastInterface(const char *interface_addr) {
  int r = uv_udp_set_multicast_interface(getUdp(), interface_addr);
  LOG_IF_ERROR(r);
  return r;
}

int UdpI::setBroadcast(int on) {
  int r = uv_udp_set_broadcast(getUdp(), on);
  LOG_IF_ERROR(r);
  return r;
}

int UdpI::setTtl(int ttl) {
  int r = uv_udp_set_ttl(getUdp(), ttl);
  LOG_IF_ERROR(r);
  return r;
}

int UdpI::send(const BufT bufs[], unsigned int nbufs,
               const struct sockaddr *addr) {
  auto req = allocAnyReq<uv_udp_send_t>(bufs, nbufs);
  int r = uv_udp_send((uv_udp_send_t *)req, getUdp(), bufs, nbufs, addr,
                      UdpI::Impl::send_callback);
  if (r) {
    freeAnyReq(req);
    LOG_IF_ERROR(r);
  }
  return r;
}

int UdpI::trySend(const BufT bufs[], unsigned int nbufs,
                  const struct sockaddr *addr) {
  int r = uv_udp_try_send(getUdp(), bufs, nbufs, addr);
  LOG_IF_ERROR(r);
  return r;
}

int UdpI::recvStart() {
  int r = uv_udp_recv_start(getUdp(), HandleI::Impl::alloc_callback,
                            Impl::recv_callback);
  LOG_IF_ERROR(r);
  return r;
}

int UdpI::recvStop() {
  int r = uv_udp_recv_stop(getUdp());
  LOG_IF_ERROR(r);
  return r;
}

int UdpI::getSendQueueSize() const {
  int r = uv_udp_get_send_queue_size(getUdp());
  LOG_IF_ERROR(r);
  return r;
}

int UdpI::getSendQueueCount() const {
  int r = uv_udp_get_send_queue_count(getUdp());
  LOG_IF_ERROR(r);
  return r;
}

void UdpI::sendCallback(const UdpI::SendCallback &cb) {
  _impl->_sendCallback = cb;
}

UdpI::SendCallback UdpI::sendCallback() const { return _impl->_sendCallback; }

void UdpI::recvCallback(const UdpI::RecvCallback &cb) {
  _impl->_recvCallback = cb;
}

UdpI::RecvCallback UdpI::recvCallback() const { return _impl->_recvCallback; }

// --

uv_handle_t *UdpT::getHandle() const { return (uv_handle_t *)&_udp; }

uv_udp_t *UdpT::getUdp() const { return (uv_udp_t *)&_udp; }

UdpT::UdpT(LoopT *loop) {
  int r = uv_udp_init(loop->get(), &_udp);
  LOG_IF_ERROR_EXIT(r);

  uv_handle_set_data(getHandle(), this);
}

UdpT::UdpT(LoopT *loop, unsigned int flags) {
  int r = uv_udp_init_ex(loop->get(), &_udp, flags);
  LOG_IF_ERROR_EXIT(r);

  uv_handle_set_data(getHandle(), this);
}

UdpT::~UdpT() {}

// --

struct AsyncI::Impl {
  AsyncCallback _asyncCallback;

  static void async_callback(uv_async_t *handle);
};

void AsyncI::Impl::async_callback(uv_async_t *handle) {
  auto p = (AsyncI *)uv_handle_get_data((uv_handle_t *)handle);
  if (p->_impl->_asyncCallback) {
    p->_impl->_asyncCallback();
  }
}

// --

AsyncI::AsyncI() : _impl(std::make_unique<AsyncI::Impl>()) {}

AsyncI::~AsyncI() {}

int AsyncI::send() {
  int r = uv_async_send(getAsync());
  LOG_IF_ERROR(r);
  return r;
}

void AsyncI::asyncCallback(const AsyncI::AsyncCallback &cb) {
  _impl->_asyncCallback = cb;
}

AsyncI::AsyncCallback AsyncI::asyncCallback() const {
  return _impl->_asyncCallback;
}

// --

uv_handle_t *AsyncT::getHandle() const { return (uv_handle_t *)&_async; }

uv_async_t *AsyncT::getAsync() const { return (uv_async_t *)&_async; }

AsyncT::AsyncT(LoopT *loop) {
  int r = uv_async_init(loop->get(), &_async, AsyncI::Impl::async_callback);
  LOG_IF_ERROR_EXIT(r);

  uv_handle_set_data(getHandle(), this);
}

AsyncT::~AsyncT() {}
