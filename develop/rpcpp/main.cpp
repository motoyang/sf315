#include <iostream>
#include <nanolog/nanolog.hpp>

#include "anyarg.h"
#include "server.h"
#include "client.h"

// --

int main(int argc, char **argv) {

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
  if (opt.is_true('h')
    || ((opt.is_true('s') && opt.is_true('d'))
    || (opt.is_true('s') && opt.is_true('c'))
    || (opt.is_true('d') && opt.is_true('c'))
    || (!opt.is_true('d') && !opt.is_true('s') && !opt.is_true('c')))) {
    std::cout << opt.auto_usage();
    return 0;
  }

  // 根据参数，做不同的事情
  int r = 0;
  if (opt.is_true('d')) {
    nanolog::initialize(nanolog::GuaranteedLogger(), "/tmp/", "rpcppd", 1, 3);
    LOG_INFO << "rpcpp started as a deamon services.";

    r = startDaemon(opt);
  }
  if (opt.is_true('s')) {
    nanolog::initialize(nanolog::GuaranteedLogger(), "/tmp/", "rpcpps", 1, 3);
    LOG_INFO << "rpcpp started as a server process.";

    r = startServer(opt);
  }
  if (opt.is_true('c')) {
    nanolog::initialize(nanolog::GuaranteedLogger(), "/tmp/", "rpcppc", 1, 3);
    LOG_INFO << "rpcpp started as a client program.";

    r = rpcpp2::client::startClient(opt);
  }
  LOG_INFO << "rpcpp quit with return code: " << r;

  return 0;
}
