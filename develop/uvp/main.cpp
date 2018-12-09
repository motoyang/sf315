#include <iostream>

#include <nanolog/nanolog.hpp>
#include <misc.hpp>
#include <req.hpp>
#include <uv.hpp>
#include <gangway.hpp>

// #include <unistd.h>

#include "sighandler.h"
#include "client.h"
#include "server.h"

void sig16_handler(int signum, siginfo_t *info, void *myact) {
  printf("the int value is %d \n", info->si_int);
}

// --

int main(int argc, char *argv[]) {
  const unsigned int log_file_count = 5;
  nanolog::initialize(nanolog::GuaranteedLogger(), "/tmp/", argv[0], 1,
                      log_file_count);
  LOG_INFO << "libuv version: " << Version().str();
  std::cout << "libuv version: " << Version().str() << std::endl;
  auto loop = LoopT::defaultLoop();
  // auto loop = std::make_unique<LoopT>();
/*
  if (argc == 4) {
    pid_t pid = atoi(argv[1]);
    int sig = atoi(argv[2]);
    int tag = atoi(argv[3]);
    std::cout << "sig_send(" << pid << ", " << sig << ", " << tag << ");"
              << std::endl;
    sig_send(pid, sig, tag);
  } else {
    pid_t pid = getpid();
    std::cout << "pid = " << pid << std::endl;
    int sig = atoi(argv[1]);
    std::cout << "sig_capture(" << sig << ");" << std::endl;
    sig_capture(sig, sig16_handler);
  }
*/

  // tcp_client(loop.get());
  tcp_server(loop.get());

  loop->close();
  return 0;
}
