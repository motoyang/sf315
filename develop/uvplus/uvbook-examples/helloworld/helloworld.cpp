#include <iostream>
#include <memory>

#include <uvp.hpp>

int main(int argc, char *argv[]) {
  uvp::initialize("./", "helloworld", 1, 3);
  
  auto l = std::make_unique<uvp::LoopObject>();
  std::cout << "Now quitting." << std::endl;
  l->run(UV_RUN_DEFAULT);
  l->close();

  LOG_INFO << "quit with return code: " << 0;
  return 0;
}
