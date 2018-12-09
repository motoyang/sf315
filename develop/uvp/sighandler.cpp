#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include "sighandler.h"

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

