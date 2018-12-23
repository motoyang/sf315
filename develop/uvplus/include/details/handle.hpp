#pragma once

#include <functional>

#include <details/types.hpp>
#include <details/misc.hpp>
#include <details/eventloop.hpp>

// --

namespace uvp {

class Handle {
public:
  using Type = uv::HandleType;

  using AllocCallback = std::function<void(size_t, uv::BufT *)>;
  using FreeCallback = std::function<void(uv::BufT)>;
  using CloseCallback = std::function<void()>;

protected:
  struct Impl {
    void *_data = nullptr;
    uv::BufT _buffer = {0};

    AllocCallback _allocCallback;
    FreeCallback _freeCallback;
    CloseCallback _closeCallback;

    virtual ~Impl() {
      if (_buffer.base) {
        freeBuf(_buffer);
      }
    }
  };
  Impl _impl;

  virtual uv::HandleT *handle() const = 0;

  static void alloc_callback(uv::HandleT *handle, size_t suggested_size,
                             uv::BufT *buf) {
    auto p = (Handle*)uv_handle_get_data(handle);
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
    auto p = (Handle*)uv_handle_get_data(handle);
    if (p->_impl._closeCallback) {
      p->_impl._closeCallback();
    }
  }

public:
  static size_t size(Type t) { return uv_handle_size(t); }

  static const char *typeName(Type t) { return uv_handle_type_name(t); }

  Handle() {}

  virtual ~Handle() {}

  bool isActive() const { return uv_is_active(handle()); }

  bool isClosing() const { return uv_is_closing(handle()); }

  void close(const CloseCallback &cb) {
    _impl._closeCallback = cb;
    uv_close(handle(), close_callback);
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
    return (Loop *)loop;
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
  using IdleCallback = std::function<void()>;

protected:
  struct Impl {
    IdleCallback _idleCallback;
  };
  Impl _impl;

  static void idle_callback(uv::IdleT *idle) {
    auto p = (Idle *)uv_handle_get_data((uv::HandleT*)idle);
    if (p->_impl._idleCallback) {
      p->_impl._idleCallback();
    } else {
      UVP_ASSERT(false);
    }
  }

  virtual uv::IdleT *idle() const = 0;

public:
  Idle() {}
  virtual ~Idle() {}

  int start(const IdleCallback &cb) {
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

protected:
  virtual uv::HandleT *handle() const override {
    return (uv::HandleT *)&_idle;
  }

  virtual uv::IdleT *idle() const override { return &_idle; }

public:
  IdleObject(Loop *loop) {
    int r = uv_idle_init(loop->loop(), idle());
    LOG_IF_ERROR_EXIT(r);
    uv_handle_set_data(handle(), this);
  }
  virtual ~IdleObject() {}
};

// --

} // namespace uvp
