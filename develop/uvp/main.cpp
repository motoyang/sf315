#include <iostream>

#include <utilites.hpp>
#include <misc.hpp>
#include <uv.hpp>

// #include "sighandler.h"
#include "anyarg.h"
#include "client.h"
#include "server.h"

// void sig16_handler(int signum, siginfo_t *info, void *myact) {
//   printf("the int value is %d \n", info->si_int);
// }

// --

int main(int argc, char *argv[]) {
  Anyarg opt;
  opt.add_flag("help", 'h', "Display help information.");
  opt.add_flag("daemon", 'd', "running as a daemon services.");
  opt.add_flag("server", 's', "running as a server process.");
  opt.add_flag("client", 'c', "running as client program");
  opt.add_option_str("url", 'u', "ipc:///tmp/rpcpp.ipc", "url address.");
  opt.add_option_str("case", 0, "reqrep", "which case will be started.");

  // parsing command line, collect command line arguments
  opt.parse_argv(argc, argv);

  // generate formatted usage information for all options
  if (opt.is_true('h') ||
      ((opt.is_true('s') && opt.is_true('d')) ||
       (opt.is_true('s') && opt.is_true('c')) ||
       (opt.is_true('d') && opt.is_true('c')) ||
       (!opt.is_true('d') && !opt.is_true('s') && !opt.is_true('c')))) {
    std::cout << opt.auto_usage();
    return 0;
  }

  auto loop = LoopT::defaultLoop();
  // auto loop = std::make_unique<LoopT>();

  // 根据参数，做不同的事情
  unsigned int log_file_count = 5;
  int r = 0;
  if (opt.is_true('d')) {
    nanolog::initialize(nanolog::GuaranteedLogger(), "/tmp/", "uvpd", 1,
                        log_file_count);
    LOG_INFO << "libuv version: " << Version().str();
    std::cout << "libuv version: " << Version().str() << std::endl;

    // r = startDaemon(opt);
  }
  if (opt.is_true('s')) {
    nanolog::initialize(nanolog::GuaranteedLogger(), "/tmp/", "uvps", 1,
                        log_file_count);
    LOG_INFO << "libuv version: " << Version().str();
    std::cout << "libuv version: " << Version().str() << std::endl;

    tcp_server(loop.get());
  }
  if (opt.is_true('c')) {
    nanolog::initialize(nanolog::GuaranteedLogger(), "/tmp/", "uvpc", 1,
                        log_file_count);
    LOG_INFO << "libuv version: " << Version().str();
    std::cout << "libuv version: " << Version().str() << std::endl;

    tcp_client(loop.get());
  }
  LOG_INFO << "uvp quit with return code: " << 0;
  /*
    const unsigned int log_file_count = 5;
    nanolog::initialize(nanolog::GuaranteedLogger(), "/tmp/", argv[0], 1,
                        log_file_count);
    LOG_INFO << "libuv version: " << Version().str();
    std::cout << "libuv version: " << Version().str() << std::endl;
  */
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

  // tcp_client(loop.get());`
  // tcp_server(loop.get());

  loop->close();
  return 0;
}
