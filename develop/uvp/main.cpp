#include <iostream>

#include <nanolog/nanolog.hpp>
#include <misc.hpp>
#include <uv.hpp>

#include "client.h"
#include "server.h"

// --

int main(int argc, char *argv[]) {
  const unsigned int log_file_count = 5;
  nanolog::initialize(nanolog::GuaranteedLogger(), "/tmp/", "uvp", 1,
                      log_file_count);

  std::cout << "libuv version: " << Version().str() << std::endl;
  auto loop = LoopT::defaultLoop();
  // auto loop = std::make_unique<LoopT>();

  // tcp_client(loop.get());
  tcp_server(loop.get());
  
  loop->close();
  return 0;
}
