#pragma once

typedef void (*sig_action) (int, siginfo_t *, void *);

int sig_capture(int sig, sig_action action);
int sig_send(pid_t pid, int sig, int tag);
