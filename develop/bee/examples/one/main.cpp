#include <iostream>

#include "src/nanolog.hpp"
#include "src/memoryimpl.h"
#include "src/packimpl.h"
#include "src/cryptography.h"
#include "src/cryptocenter.h"
#include "src/versionsupported.h"
#include "src/recordlayer.h"
#include "src/channel.h"
#include "src/server.h"
#include "src/client.h"

int main(int argc, char* argv[]) {
  nlog::Logger(nlog::NonGuaranteedLogger(1), "./", argv[0]);
  MemoryInterface::set(std::make_shared<MemoryManager>());
  secure::initialize();

  Client c;
  c.start();
  Server s;
  s.run();

  return EXIT_SUCCESS;
}