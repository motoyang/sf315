#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include <uvplus.hpp>

#include "calltrace.h"
#include "sighandler.h"

// --

// SIGUSR2 handler
void sig12_handler(int signum, siginfo_t *info, void *myact) {
  LOG_INFO << "received signal: " << signum << " and tag: " << info->si_int;
  // if (info->si_int == (int)uvplus::TcpAcceptor::NotifyTag::NT_CLOSE) {
  //   g_business->stop();
  // }
  // g_acceptor->notify(info->si_int);
}

// SIGPIPE handler
void sig13_ignore() {
  struct sigaction sa;
  sa.sa_handler = SIG_IGN; //设定接受到指定信号后的动作为忽略
  sa.sa_flags = 0;
  if (sigemptyset(&sa.sa_mask) == -1 ||   //初始化信号集为空
      sigaction(SIGPIPE, &sa, 0) == -1) { //屏蔽SIGPIPE信号
    perror("failed to ignore SIGPIPE; sigaction");
    exit(EXIT_FAILURE);
  }
}
// --

int sig_capture(int sig, sig_action action) {
  struct sigaction act;
  sigemptyset(&act.sa_mask);
  act.sa_sigaction = action;
  act.sa_flags = SA_SIGINFO;
  return sigaction(sig, &act, NULL);
}

int sig_send(pid_t pid, int sig, int tag) {
  union sigval mysigval;
  mysigval.sival_int = tag;
  return sigqueue(pid, sig, mysigval);
}

void coredump_catch(int singal) {
  const size_t max = 30;
  void *buffer[max];
  std::ostringstream oss;
  std::cout << "core dump due to singal: " << signal << std::endl;
  dumpBacktrace(std::cout, buffer, captureBacktrace(buffer, max));
}

int sig_coredump() {
  // 其中saveBackTrace为回调函数
  signal(SIGSEGV, coredump_catch);
  signal(SIGILL, coredump_catch);
  signal(SIGFPE, coredump_catch);
  signal(SIGABRT, coredump_catch);
  signal(SIGTERM, coredump_catch);
  signal(SIGKILL, coredump_catch);
  signal(SIGXFSZ, coredump_catch);

  // block SIGINT to all child process:
  sigset_t bset, oset;
  sigemptyset(&bset);
  sigaddset(&bset, SIGINT);
  // equivalent to sigprocmask
  //设置信号为阻塞信号
  if (pthread_sigmask(SIG_BLOCK, &bset, &oset) != 0) {
    printf("set thread signal mask error!");
    return 0;
  }
}

int sig_coredump2() { signal(SIGSEGV, print_stack_frames); }