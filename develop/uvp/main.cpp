#include <stdio.h>
#include <uv.h>

#include <iostream>

#include <nanolog/nanolog.hpp>

#include <misc.hpp>
#include <uv.hpp>

// --

void test_idle(const std::unique_ptr<LoopT> &loop) {
  int counter = 0;
  IdleT idler(loop);
  idler.start([&idler, &counter]() {
    counter++;
    if (counter >= 10e6) {
      idler.stop();
      idler.close([]() { std::cout << "idle closed." << std::endl; });
    }
  });
  idler.data(&counter);

  std::cout << "Idling..." << std::endl;
  loop->run(UV_RUN_DEFAULT);

  std::cout << "counter: " << *(int *)idler.data() << std::endl;
  std::cout << "idler.isActive(): " << idler.isActive() << std::endl;
  std::cout << "idler.isClosing(): " << idler.isClosing() << std::endl;
  std::cout << "idler.hasRef(): " << idler.hasRef() << std::endl;
  std::cout << "ldler.type(): " << IdleT::typeName(idler.type()) << std::endl;
}

// --

void test_timer(const std::unique_ptr<LoopT> &loop) {
  int counter = 0;
  TimerT timer(loop);
  timer.start([&timer, &counter]() {
    ++counter;
    if (counter > 10) {
      timer.stop();
      timer.close([]() { std::cout << "timer closed." << std::endl; });
    }
  }, 1000, 200);
  timer.data(&counter);

  std::cout << "Timer..." << std::endl;
  loop->run(UV_RUN_DEFAULT);

  std::cout << "counter: " << *(int *)timer.data() << std::endl;
  std::cout << "timer.isActive(): " << timer.isActive() << std::endl;
  std::cout << "timer.isClosing(): " << timer.isClosing() << std::endl;
  std::cout << "timer.hasRef(): " << timer.hasRef() << std::endl;
  std::cout << "ldler.type(): " << IdleT::typeName(timer.type()) << std::endl;
}

int main() {
  const unsigned int log_file_count = 5;
  nanolog::initialize(nanolog::GuaranteedLogger(), "/tmp/", "uvp", 1,
                      log_file_count);

  std::cout << "libuv version: " << Version().str() << std::endl;
  // auto loop = LoopT::defaultLoop();
  auto loop = std::make_unique<LoopT>();

  // test_idle(loop);
  test_timer(loop);
  
  loop->close();
  return 0;
}