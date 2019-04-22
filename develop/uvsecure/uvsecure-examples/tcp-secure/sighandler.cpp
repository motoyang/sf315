#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include <uvplus.hpp>

#include "secureacceptor.h"
#include "business.h"
#include "sighandler.h"

extern uvplus::TcpAcceptor *g_acceptor;
extern Business *g_business;

// --

// SIGUSR2 handler
void sig12_handler(int signum, siginfo_t *info, void *myact) {
  LOG_INFO << "received signal: " << signum << " and tag: " << info->si_int;
  if (info->si_int ==
      (int)uvplus::TcpAcceptor::NotifyTag::NT_CLOSE) {
    g_business->stop();
  }
  g_acceptor->notify(info->si_int);
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
