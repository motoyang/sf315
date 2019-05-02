#pragma once

typedef void (*sig_action) (int, siginfo_t *, void *);

int sig_capture(int sig, sig_action action);
int sig_send(pid_t pid, int sig, int tag);
void sig12_handler(int signum, siginfo_t *info, void *myact);
void sig13_ignore();
int sig_coredump();
int sig_coredump2();
