#pragma once

#include <functional>
#include <memory>
#include <initializer_list>

#include <uv.h>
#include <types.hpp>

// --

class HandleI;
class LoopI {
protected:
  class Impl;
  std::unique_ptr<Impl> _impl;

public:
  using RunMode = uv_run_mode;
  using LoopOperation = uv_loop_option;

  using WalkCallback = std::function<void(HandleI *, void *)>;

  static size_t size();

  virtual uv_loop_t *getLoop() const = 0;

  LoopI();
  LoopI(uv_loop_t *l);
  virtual ~LoopI();

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
};

class LoopT : public LoopI {
  uv_loop_t *_loop;

public:
  static std::unique_ptr<LoopI> defaultLoop();

  virtual uv_loop_t *getLoop() const override;

  LoopT();
  virtual ~LoopT();

private:
  friend std::unique_ptr<LoopI> LoopT::defaultLoop();
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
  using FreeCallback = std::function<void(BufT)>;
  using CloseCallback = std::function<void()>;

  static size_t size(Type t);
  static const char *typeName(Type t);

  HandleI();
  virtual ~HandleI();

  bool isActive() const;
  bool isClosing() const;
  // void close(CloseCallback &&cb);
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
  LoopI *loop() const;
  void *data() const;
  void *data(void *data);
  Type type() const;

  // extend functions
  void setDefaultSize(size_t bufSize, size_t queueSize);
  void allocCallback(const AllocCallback &cb);
  AllocCallback allocCallback() const;
  void freeCallback(const FreeCallback &cb);
  FreeCallback freeCallback() const;
  void closeCallback(const CloseCallback &cb);
  CloseCallback closeCallback() const;
  void close();
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

  int start();
  int stop();

  // extend functions
  IdleCallback idleCallback() const;
  void idleCallback(const IdleCallback &cb);
};

class IdleT : public IdleI {
  uv_idle_t _idle;

protected:
  virtual uv_handle_t *getHandle() const override;
  virtual uv_idle_t *getIdle() const override;

public:
  IdleT(LoopI *loop);
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

  int start(uint64_t timeout, uint64_t repeat);
  int stop();
  int again();
  void repeat(uint64_t repeat);
  uint64_t repeat() const;

  // extend functions
  void timerCallback(const TimerCallback &cb);
  TimerCallback timerCallback() const;
};

class TimerT : public TimerI {
  uv_timer_t _timer;

protected:
  virtual uv_handle_t *getHandle() const override;
  virtual uv_timer_t *getTimer() const override;

public:
  TimerT(LoopI *loop);
  virtual ~TimerT();
};

// --

class StreamT;
class StreamI : public HandleI {
protected:
  class Impl;
  std::unique_ptr<Impl> _impl;

  virtual uv_stream_t *getStream() const = 0;

public:
  using ConnectCallback = std::function<void(int)>;
  using ReadCallback = std::function<void(ssize_t, const BufT *)>;
  using WriteCallback = std::function<void(int, BufT[], int)>;
  using ShutdownCallback = std::function<void(int)>;
  using ConnectionCallback = std::function<void(int)>;

  StreamI();
  virtual ~StreamI();

  int accept(StreamI *client);
  int readStop();
  int tryWrite(BufT bufs[], unsigned int nbufs);
  int isReadable() const;
  int isWritable() const;
  int setBlocking(int blocking);
  int getWriteQueueSize() const;

  // extend functions
  void shutdownCallback(const ShutdownCallback &cb);
  ShutdownCallback shutdownCallback() const;
  void connectionCallback(const ConnectionCallback &cb);
  ConnectionCallback connectionCallback() const;
  void readCallback(const ReadCallback &cb);
  ReadCallback readCallback() const;
  void writeCallback(const WriteCallback &cb);
  WriteCallback writeCallback() const;

  int shutdown();
  int listen(int backlog);
  int readStart();
  int write(const BufT bufs[], unsigned int nbufs);
  int write2(const BufT bufs[], unsigned int nbufs, StreamI *sendstream);
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
  int open(File file);
  int bind(const char *name);
  void connect(const char *name);
  int getSockname(char *buffer, size_t *size) const;
  int getPeername(char *buffer, size_t *size) const;
  void pendingInstances(int count);
  int pendingCount();
  HandleI::Type pendingType();
  int chmod(int flags);

  // extend functions
  void connectCallback(const ConnectCallback &cb);
  ConnectCallback connectCallback() const;
};

class PipeT : public PipeI {
  uv_pipe_t _pipe;

protected:
  virtual uv_handle_t *getHandle() const override;
  virtual uv_stream_t *getStream() const override;
  virtual uv_pipe_t *getPipe() const override;

public:
  PipeT(LoopI *loop, int ipc);
  ~PipeT();
};

// --

class TcpI : public StreamI {
protected:
  virtual uv_tcp_t *getTcp() const = 0;

public:
  int open(OsSock sock);
  int nodelay(int enable);
  int keepalive(int enable, unsigned int delay);
  int simultaneousAcepts(int enable);
  int bind(const struct sockaddr *addr, unsigned int flags);
  int getsockname(struct sockaddr *name, int *namelen);
  int getpeername(struct sockaddr *name, int *namelen);
  int connect(const struct sockaddr *addr);

  // extend functions
  void connectCallback(const ConnectCallback &cb);
  ConnectCallback connectCallback() const;
  int reinit();
};

class TcpT : public TcpI {
  uv_tcp_t _tcp;

protected:
  virtual uv_handle_t *getHandle() const override;
  virtual uv_stream_t *getStream() const override;
  virtual uv_tcp_t *getTcp() const override;

public:
  TcpT(LoopI *loop);
  TcpT(LoopI *loop, unsigned int flags);
  virtual ~TcpT();
};

// --

class TtyI : public StreamI {
protected:
  virtual uv_tty_t *getTty() const = 0;

public:
  using TtyMode = uv_tty_mode_t;

  static int resetMode();

  int setMode(TtyMode mode);
  int getWinsize(int *width, int *height);
};

class TtyT : public TtyI {
  uv_tty_t _tty;

protected:
  virtual uv_handle_t *getHandle() const override;
  virtual uv_stream_t *getStream() const override;
  virtual uv_tty_t *getTty() const override;

public:
  TtyT(LoopI *loop, File fd, int unused);
  virtual ~TtyT();
};

// --

class UdpI : public HandleI {
protected:
  class Impl;
  std::unique_ptr<Impl> _impl;

  virtual uv_udp_t *getUdp() const = 0;

public:
  using MemberShip = uv_membership;

  using SendCallback = std::function<void(int, BufT *, int)>;
  using RecvCallback = std::function<void(ssize_t, const BufT *,
                                          const struct sockaddr *, unsigned)>;

  UdpI();
  virtual ~UdpI();

  int open(OsSock sock);
  int bind(const struct sockaddr *addr, unsigned int flags);
  int getsockname(struct sockaddr *name, int *namelen) const;
  int setMembership(const char *multicast_addr, const char *interface_addr,
                    MemberShip membership);
  int setMulticastLoop(int on);
  int setMulticastTtl(int ttl);
  int setMulticastInterface(const char *interface_addr);
  int setBroadcast(int on);
  int setTtl(int ttl);
  int send(const BufT bufs[], unsigned int nbufs, const struct sockaddr *addr);
  int trySend(const BufT bufs[], unsigned int nbufs,
              const struct sockaddr *addr);
  int recvStart();
  int recvStop();
  int getSendQueueSize() const;
  int getSendQueueCount() const;

  // extend functions
  void sendCallback(const SendCallback &cb);
  SendCallback sendCallback() const;
  void recvCallback(const RecvCallback &cb);
  RecvCallback recvCallback() const;
};

class UdpT : public UdpI {
  uv_udp_t _udp;

protected:
  virtual uv_handle_t *getHandle() const;
  virtual uv_udp_t *getUdp() const;

public:
  UdpT(LoopI *loop);
  UdpT(LoopI *loop, unsigned int flags);
  virtual ~UdpT();
};

// --

class AsyncI : public HandleI {
protected:
  class Impl;
  std::unique_ptr<Impl> _impl;

  virtual uv_async_t *getAsync() const = 0;

public:
  using AsyncCallback = std::function<void()>;

  AsyncI();
  virtual ~AsyncI();

  int send();

  void asyncCallback(const AsyncCallback &cb);
  AsyncCallback asyncCallback() const;
};

class AsyncT : public AsyncI {
  uv_async_t _async;

protected:
  virtual uv_handle_t *getHandle() const;
  virtual uv_async_t *getAsync() const;

public:
  AsyncT(LoopI *loop);
  virtual ~AsyncT();
};

// --

class SignalI : public HandleI {
protected:
  class Impl;
  std::unique_ptr<Impl> _impl;

  virtual uv_signal_t *getSignal() const = 0;

public:
  using SignalCallback = std::function<void(int)>;

  SignalI();
  virtual ~SignalI();

  int start(int signum);
  int startOneshort(int signum);
  int stop();
};

class SignalT : public SignalI {
  uv_signal_t _signal;

protected:
  virtual uv_handle_t *getHandle() const;
  virtual uv_signal_t *getSignal() const;

public:
  SignalT(LoopI *loop);
  virtual ~SignalT();
};

// --

class ProcessI : public HandleI {
protected:
  // class Impl;
  // std::unique_ptr<Impl> _impl;

  virtual uv_process_t *getProcess() const = 0;

public:
  using Options = uv_process_options_t;
  using Pid = uv_pid_t;

  static void disableStdioInheritance();
  static int kill(int pid, int signum);

  ProcessI() = default;
  virtual ~ProcessI() = default;

  int spawn(LoopI *loop, const Options *options);
  int kill(int signum);
  Pid getPid() const;
};

class ProcessT : public ProcessI {
  uv_process_t _process;

protected:
  virtual uv_handle_t *getHandle() const;
  virtual uv_process_t *getProcess() const;

public:
  ProcessT() = default;
  virtual ~ProcessT() = default;
};
