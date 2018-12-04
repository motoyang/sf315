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
};

// --

std::unique_ptr<LoopT> LoopT::defaultLoop() {
  uv_loop_t *loop = uv_default_loop();
  return std::unique_ptr<LoopT>(new LoopT(loop));
}

LoopT::LoopT() : _impl(std::make_unique<LoopT::Impl>()) {
  uv_loop_init(_impl->_loop.get());
}

LoopT::LoopT(uv_loop_t *l) : _impl(std::make_unique<LoopT::Impl>(l)) {}

LoopT::~LoopT() {}

int LoopT::close() { return uv_loop_close(_impl->_loop.get()); }

int LoopT::run(LoopT::RunMode mode) { return uv_run(_impl->_loop.get(), mode); }

uint64_t LoopT::now() const {
  return uv_now(get());
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
  HandleT *h = (HandleT *)uv_handle_get_data(handle);
  h->_impl->_allocCallback(handle, suggested_size, buf);
}

void HandleI::Impl::close_callback(uv_handle_t *handle) {
  HandleT *h = (HandleT *)uv_handle_get_data(handle);
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

std::unique_ptr<LoopT> HandleI::loop() const {
  uv_loop_t *loop = uv_handle_get_loop(getHandle());
  return std::unique_ptr<LoopT>(new LoopT(loop));
}

void *HandleI::data() const { return _impl->_data; }

void *HandleI::data(void *data) { return _impl->_data = data; }

HandleI::Type HandleI::type() const { return uv_handle_get_type(getHandle()); }

// --

uv_handle_t *HandleT::getHandle() const { return (uv_handle_t *)&_handle; }

HandleT::HandleT() {}

HandleT::~HandleT() {}

// --

struct IdleI::Impl {
  IdleCallback _idleCallback;
  static void idle_callback(uv_idle_t *idle);
};

void IdleI::Impl::idle_callback(uv_idle_t *idle) {
  auto p = (IdleT *)uv_handle_get_data((uv_handle_t *)idle);
  p->_impl->_idleCallback();
}

// --

IdleI::IdleI() : _impl(std::make_unique<IdleI::Impl>()) {}

IdleI::~IdleI() {}

int IdleI::start(IdleCallback &&cb) {
  _impl->_idleCallback = std::forward<IdleCallback>(cb);
  int r = uv_idle_start(getIdle(), IdleI::Impl::idle_callback);
  LOG_IF_ERROR_EXIT(r);
  return r;
}

int IdleI::stop() {
  int r = uv_idle_stop(getIdle());
  LOG_IF_ERROR_EXIT(r);
  return r;
}

// --

uv_handle_t *IdleT::getHandle() const { return (uv_handle_t *)&_idle; }

uv_idle_t *IdleT::getIdle() const { return (uv_idle_t *)&_idle; }

IdleT::IdleT(const std::unique_ptr<LoopT> &loop) {
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
  auto p = (TimerT *)uv_handle_get_data((uv_handle_t *)handle);
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
  LOG_IF_ERROR_EXIT(r);
  return r;
}

int TimerI::stop() {
  int r = uv_timer_stop(getTimer());
  LOG_IF_ERROR_EXIT(r);
  return r;
}

int TimerI::again() {
  int r = uv_timer_again(getTimer());
  LOG_IF_ERROR_EXIT(r);
  return r;
}

void TimerI::repeat(uint64_t repeat) {
  uv_timer_set_repeat(getTimer(), repeat);
}

uint64_t TimerI::repeat() const { return uv_timer_get_repeat(getTimer()); }

// --

uv_handle_t *TimerT::getHandle() const { return (uv_handle_t *)&_timer; }

uv_timer_t *TimerT::getTimer() const { return (uv_timer_t *)&_timer; }

TimerT::TimerT(const std::unique_ptr<LoopT> &loop) {
  int r = uv_timer_init(loop->get(), &_timer);
  LOG_IF_ERROR_EXIT(r);

  uv_handle_set_data(getHandle(), this);
}

TimerT::~TimerT() {}

// --
