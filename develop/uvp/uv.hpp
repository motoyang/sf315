#pragma once

#include <uv.h>

#include <functional>
#include <memory>
#include <initializer_list>

// #include <req.hpp>

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

  using WalkCallback = std::function<void(HandleI *, void *)>;

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
  LoopT(uv_loop_t *l);
};

// --

class HandleI {
protected:
  class Impl;
  std::unique_ptr<Impl> _impl;

  virtual uv_handle_t *getHandle() const = 0;

public:
  using Type = uv_handle_type;

  using AllocCallback = std::function<void(size_t, BufT *)>;
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
  LoopT *loop() const;
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
  IdleT(LoopT *loop);
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
  TimerT(LoopT *loop);
  virtual ~TimerT();
};

// --

class StreamT;
class StreamI : public HandleI {
  class Impl;
  std::unique_ptr<Impl> _impl;

protected:
  virtual uv_stream_t *getStream() const = 0;

public:
  
  using ReadCallback = std::function<void(ssize_t, const BufT *)>;
  using WriteCallback = std::function<void(uv_write_t*, int)>;
  using ConnectCallback = std::function<void(int)>;
  using ShutdownCallback = std::function<void(int)>;
  using ConnectionCallback = std::function<void(int)>;

  StreamI();
  virtual ~StreamI();

  int shutdown(uv_shutdown_t *req, ShutdownCallback &&cb);
  int listen(int backlog, ConnectionCallback &&cb);
  int accept(StreamT *client);
  int readStart(AllocCallback &&alloc, ReadCallback &&cb);
  int readStop();
  int write(uv_write_t* req, BufT bufs[], unsigned int nbufs, WriteCallback &&cb);
  int write2(uv_write_t* req, BufT bufs[], unsigned int nbufs, StreamT *sendstream,
             WriteCallback &&cb);
  int tryWrite(BufT bufs[], unsigned int nbufs);
  int isReadable() const;
  int isWritable() const;
  int setBlocking(int blocking);
  int getWriteQueueSize() const;
};

class StreamT : public StreamI {
  friend class StreamI;
  uv_stream_t _stream;

protected:
  virtual uv_handle_t *getHandle() const;
  virtual uv_stream_t *getStream() const;

public:
  StreamT();
  virtual ~StreamT();
};

// --

class PipeI : public StreamI {
protected:
  // class Impl;
  // std::unique_ptr<Impl> _impl;

  virtual uv_pipe_t *getPipe() const = 0;

public:
  using File = uv_file;
  
  int open(File file);
  int bind(const char* name);
  // int connect(ConnectT* req, const char* name, ConnectCallback&& cb);
  // int getSockname(char* buffer, size_t* size) const;
  // int getPeername(char* buffer, size_t* size) const;
  // int pendingInstances(int count);
  // int pendingCount(PipeT* pipe);
  // HandleI::Type pendingType();
  // int chmod();
};

class PipeT : public PipeI {
  uv_pipe_t _pipe;

protected:
  virtual uv_handle_t *getHandle() const override;
  virtual uv_stream_t* getStream() const override;
  virtual uv_pipe_t *getPipe() const override;

public:
  PipeT(LoopT *loop, int ipc);
  ~PipeT();
};