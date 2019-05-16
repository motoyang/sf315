#include <unistd.h>
#include <iostream>

#include <uvp.hpp>
#include <uvplus.hpp>

#include "ver.h"
#include "sighandler.h"
#include "anyarg.h"
#include "client.h"
#include "server.h"

// --

int main(int argc, char *argv[]) {
  Anyarg opt;
  opt.add_flag("version", 'v', "Display version information.");
  opt.add_flag("help", 'h', "Display help information.");
  opt.add_flag("daemon", 'd', "running as a daemon services.");
  opt.add_flag("server", 's', "running as a server process.");
  opt.add_flag("client", 'c', "running as client program");
  opt.add_flag("rsakey", 'r', "produce a pair of rsa key");
  opt.add_option_str("url", 'u', "ipc:///tmp/rpcpp.ipc", "url address.");
  opt.add_option_str("case", 0, "reqrep", "which case will be started.");
  opt.add_flag("kill", 'k', "send signal and tag to process whick id is pid.");
  opt.add_option_int("pid", 0, 0, "=PID the process id.");
  opt.add_option_int("sig", 0, 0, "=SIGNAL signal to a process.");
  opt.add_option_int("tag", 0, 0, "=TAG tag send to a process.");

  // parsing command line, collect command line arguments
  opt.parse_argv(argc, argv);

  // version information
  if (opt.is_true('v')) {
    std::cout << "uvp version: " << VERSION_MAJOR << "." << VERSION_MINOR << "."
              << VERSION_PATCH << "." << VERSION_BUILD << std::endl;
    return 0;
  }
  // generate formatted usage information for all options
  if (opt.is_true('h') ||
      ((opt.is_true('s') && opt.is_true('d')) ||
       (opt.is_true('s') && opt.is_true('c')) ||
       (opt.is_true('d') && opt.is_true('c')) ||
       (!opt.is_true('d') && !opt.is_true('s') && !opt.is_true('c') &&
        !opt.is_true('k') && !opt.is_true('r')))) {
    std::cout << opt.auto_usage();
    return 0;
  }

  unsigned int log_file_count = 5;
  std::string name_of_logger;
  if (opt.is_true('d')) {
    name_of_logger = "tcp-basic-d";
  } else if (opt.is_true('s')) {
    name_of_logger = "tcp-basic-s";
  } else if (opt.is_true('c')) {
    name_of_logger = "tcp-basic-c";
  } else {
    name_of_logger = "tcp-basic";
  }
  nlog::Logger logger(nlog::NonGuaranteedLogger(1), "/tmp/",
                      name_of_logger.c_str(), 1, log_file_count);

  // 根据参数，做不同的事情
  int r = 0;
  if (opt.is_true('k')) {
    if (opt.is_true('d') || opt.is_true('s') || opt.is_true('c') ||
        opt.is_true('k')) {
      std::cout << opt.auto_usage();
      return r;
    }
    int pid = opt.get_value_int("pid");
    int sig = opt.get_value_int("sig");
    int tag = opt.get_value_int("tag");
    if (!pid || !sig) {
      std::cout << opt.auto_usage();
      return r;
    }
    sig_send(pid, sig, tag);
    return r;
  }

  if (opt.is_true('r')) {
    if (opt.is_true('d') || opt.is_true('s') || opt.is_true('c') ||
        opt.is_true('k')) {
      std::cout << opt.auto_usage();
      return r;
    }
    return r;
  }

  sig13_ignore();
  // sig_coredump2();
  LOG_INFO << "libuv version: " << uvp::Version().str();
  std::cout << "libuv version: " << uvp::Version().str() << std::endl;
  if (opt.is_true('d')) {
    char* p = nullptr;
    *p = 3;
    // r = startDaemon(opt);
  }
  if (opt.is_true('s')) {
    LOG_INFO << "pid = " << getpid();

    // 安装信号处理，处理SIGUSR2(12)号信号
    sig_capture(SIGUSR2, sig12_handler);
    tcp_server();
  }
  if (opt.is_true('c')) {
    tcp_client();
  }

  return 0;
}
