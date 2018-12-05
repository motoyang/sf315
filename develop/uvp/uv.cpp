// #include <req.hpp>
#include <uv.hpp>
#include <utilites.hpp>

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
  void *_data = 0;
  AllocCallback _allocCallback;
  CloseCallback _closeCallback;
  static void alloc_callback(uv_handle_t *handle, size_t suggested_size,
                             BufT *buf);
  static void close_callback(uv_handle_t *handle);
};

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

HandleI::HandleI() : _impl(std::make_unique<HandleI::Impl>()) {}

HandleI::~HandleI() {}

bool HandleI::isActive() const { return uv_is_active(getHandle()); }

bool HandleI::isClosing() const { return uv_is_closing(getHandle()); }

void HandleI::close(HandleI::CloseCallback &&cb) {
  _impl->_closeCallback = std::forward<decltype(cb)>(cb);
  uv_close(getHandle(), HandleI::Impl::close_callback);
}

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

int IdleI::start(IdleCallback &&cb) {
  _impl->_idleCallback = std::forward<IdleCallback>(cb);
  int r = uv_idle_start(getIdle(), IdleI::Impl::idle_callback);
  LOG_IF_ERROR(r);
  return r;
}

int IdleI::stop() {
  int r = uv_idle_stop(getIdle());
  LOG_IF_ERROR(r);
  return r;
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
  p->_impl->_timerCallback();
}

// --

TimerI::TimerI() : _impl(std::make_unique<TimerI::Impl>()) {}

TimerI::~TimerI() {}

int TimerI::start(TimerI::TimerCallback &&cb, uint64_t timeout,
                  uint64_t repeat) {
  _impl->_timerCallback = std::forward<decltype(cb)>(cb);
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
  ReadCallback _readCallback;
  WriteCallback _writeCallback;
  ConnectCallback _connectCallback;
  ShutdownCallback _shutdownCallback;
  ConnectionCallback _connectionCallback;

  static void read_callback(uv_stream_t *stream, ssize_t nread,
                            const BufT *buf);
  static void write_callback(uv_write_t *req, int status);
  static void shutdown_callback(uv_shutdown_t *req, int status);
  static void connection_callback(uv_stream_t *server, int status);
};

void StreamI::Impl::read_callback(uv_stream_t *stream, ssize_t nread,
                                  const BufT *buf) {
  auto p = (StreamI *)uv_handle_get_data((uv_handle_t *)stream);
  p->_impl->_readCallback(nread, buf);
}

void StreamI::Impl::write_callback(uv_write_t *req, int status) {
  uv_stream_t *h = req->handle;
  auto p = (StreamI *)uv_handle_get_data((uv_handle_t *)h);
  p->_impl->_writeCallback(req, status);
}

void StreamI::Impl::shutdown_callback(uv_shutdown_t *req, int status) {
  uv_stream_t *h = req->handle;
  auto p = (StreamI *)uv_handle_get_data((uv_handle_t *)h);
  p->_impl->_shutdownCallback(status);
}

void StreamI::Impl::connection_callback(uv_stream_t *server, int status) {
  auto p = (StreamI *)uv_handle_get_data((uv_handle_t *)server);
  p->_impl->_connectionCallback(status);
}

StreamI::StreamI() : _impl(std::make_unique<StreamI::Impl>()) {}

StreamI::~StreamI() {}

int StreamI::shutdown(uv_shutdown_t *req, StreamI::ShutdownCallback &&cb) {
  _impl->_shutdownCallback = std::forward<decltype(cb)>(cb);
  int r = uv_shutdown(req, getStream(),
                      StreamI::Impl::shutdown_callback);
  LOG_IF_ERROR(r);
  return r;
}

int StreamI::listen(int backlog, StreamI::ConnectionCallback &&cb) {
  _impl->_connectionCallback = std::forward<decltype(cb)>(cb);
  int r = uv_listen(getStream(), backlog, StreamI::Impl::connection_callback);
  LOG_IF_ERROR(r);
  return r;
}

int StreamI::accept(StreamT *client) {
  int r = uv_accept(getStream(), client->getStream());
  LOG_IF_ERROR(r);
  return r;
}

int StreamI::readStart(StreamI::AllocCallback &&alloc,
                       StreamI::ReadCallback &&cb) {
  HandleI::_impl->_allocCallback = std::forward<decltype(alloc)>(alloc);
  _impl->_readCallback = std::forward<decltype(cb)>(cb);
  int r = uv_read_start(getStream(), HandleI::Impl::alloc_callback,
                        StreamI::Impl::read_callback);
  LOG_IF_ERROR(r);
  return r;
}

int StreamI::readStop() {
  int r = uv_read_stop(getStream());
  LOG_IF_ERROR(r);
  return r;
}

int StreamI::write(uv_write_t *req, BufT bufs[], unsigned int nbufs,
                   StreamI::WriteCallback &&cb) {
  _impl->_writeCallback = std::forward<decltype(cb)>(cb);
  int r = uv_write(req, getStream(), bufs, nbufs,
                   StreamI::Impl::write_callback);
  LOG_IF_ERROR(r);
  return r;
}

int StreamI::write2(uv_write_t *req, BufT bufs[], unsigned int nbufs,
                    StreamT *sendstream, WriteCallback &&cb) {
  _impl->_writeCallback = std::forward<decltype(cb)>(cb);
  int r = uv_write2(req, getStream(), bufs, nbufs,
                    sendstream->getStream(), StreamI::Impl::write_callback);
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

// --

uv_handle_t *StreamT::getHandle() const { return (uv_handle_t *)&_stream; }

uv_stream_t *StreamT::getStream() const { return (uv_stream_t *)&_stream; }

StreamT::StreamT() { uv_handle_set_data((uv_handle_t *)&_stream, this); }

StreamT::~StreamT() {}

// --

int PipeI::open(PipeI::File file) {
  int r = uv_pipe_open(getPipe(), file);
  LOG_IF_ERROR(r);
  return r;
}

int PipeI::bind(const char *name) {
  int r = uv_pipe_bind(getPipe(), name);
  LOG_IF_ERROR(r);
  return r;
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