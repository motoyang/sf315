#include <iostream>

extern "C" void initialize() {
  std::cout << "initialize libhello plugin." << std::endl;
}
