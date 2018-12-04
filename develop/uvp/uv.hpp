#include <uv.h>

#include <functional>
#include <memory>
#include <initializer_list>

// --

using BufT = uv_buf_t;
using OsFdT = uv_os_fd_t;

class HandleI;
class LoopT {
  class Impl;
  std::unique_ptr<Impl> _impl;

public:
  using RunMode = uv_run_mode;
  using LoopOperation = uv_loop_option;

  using WalkCallback = std::function<void(void *)>;

  static std::unique_ptr<LoopT> defaultLoop();
  static size_t size();

  LoopT();
  virtual ~LoopT();

  int configure(std::initializer_list<LoopOperation> options);
  int close();
  int run(RunMode mode);
  bool alive() const;
  void stop();
  // Get backend file descriptor. Only kqueue, epoll and event ports are
  // supported.
  int backendFd() const;
  int backendTimeout() const;
  uint64_t now() const;
  void updateTime();
  void walk(WalkCallback &&cb, void *arg);
  int fork();
  void *data() const;
  void *data(void *data);

  uv_loop_t *get() const;

private:
  friend std::unique_ptr<LoopT> defaultLoop();
  friend class HandleI;
  LoopT(uv_loop_t *l);
};

// --

class HandleI {
  class Impl;
  std::unique_ptr<Impl> _impl;

protected:
  virtual uv_handle_t *getHandle() const = 0;

public:
  using Type = uv_handle_type;

  using AllocCallback = std::function<void(uv_handle_t *, size_t, BufT *)>;
  using CloseCallback = std::function<void()>;

  static size_t size(Type t);
  static const char *typeName(Type t);

  HandleI();
  virtual ~HandleI();

  bool isActive() const;
  bool isClosing() const;
  void close(CloseCallback &&cb);
  void ref();
  void unref();
  bool hasRef() const;
  // This function works for TCP, pipe and UDP handles on Unix and for TCP and
  // UDP handles on Windows.
  int sendBufferSize(int *value);
  // This function works for TCP, pipe and UDP handles on Unix and for TCP and
  // UDP handles on Windows.
  int recvBufferSize(int *value);
  // The following handles are supported: TCP, pipes, TTY, UDP and poll. Passing
  // any other handle type will fail with UV_EINVAL.
  int fileno(OsFdT *fd);
  std::unique_ptr<LoopT> loop() const;
  void *data() const;
  void *data(void *data);
  Type type() const;
};

class HandleT : public HandleI {
  uv_handle_t _handle;

protected:
  virtual uv_handle_t *getHandle() const override;

public:
  HandleT();
  virtual ~HandleT();
};

// --

class IdleI : public HandleI {
  class Impl;
  std::unique_ptr<Impl> _impl;

protected:
  virtual uv_idle_t *getIdle() const = 0;

public:
  using IdleCallback = std::function<void()>;

  IdleI();
  virtual ~IdleI();

  int start(IdleCallback &&cb);
  int stop();
};

class IdleT : public IdleI {
  uv_idle_t _idle;

protected:
  virtual uv_handle_t *getHandle() const override;
  virtual uv_idle_t *getIdle() const override;

public:
  IdleT(const std::unique_ptr<LoopT> &loop);
  virtual ~IdleT();
};

// --

class TimerI : public HandleI {
  class Impl;
  std::unique_ptr<Impl> _impl;

protected:
  virtual uv_timer_t *getTimer() const = 0;

public:
  using TimerCallback = std::function<void()>;

  TimerI();
  virtual ~TimerI();

  int start(TimerCallback &&cb, uint64_t timeout, uint64_t repeat);
  int stop();
  int again();
  void repeat(uint64_t repeat);
  uint64_t repeat() const;
};

class TimerT : public TimerI {
  uv_timer_t _timer;

protected:
  virtual uv_handle_t *getHandle() const override;
  virtual uv_timer_t *getTimer() const override;

public:
  TimerT(const std::unique_ptr<LoopT> &loop);
  virtual ~TimerT();
};

// --