#include <iostream>

#include "src/nanolog.hpp"
#include "src/memoryimpl.h"

int main(int argc, char* argv[]) {
  nlog::Logger(nlog::NonGuaranteedLogger(1), "./", argv[0]);
  MemoryInterface::set(std::make_shared<MemoryManager>());

  return EXIT_SUCCESS;
}