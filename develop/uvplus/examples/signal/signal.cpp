#include <iostream>
#include <inttypes.h>

#include <uvp.hpp>

void signalHandler(uvp::Signal *handle, int signum) {
    std::cout << "Signal received: " <<  signum << std::endl;
    handle->close(nullptr);
}

void t1Worker(void* userp) {
  auto l1 = std::make_unique<uvp::LoopObject>();
  uvp::SignalObject sig1a(l1.get()), sig1b(l1.get());
  sig1a.start(signalHandler, SIGUSR1);
  sig1b.start(signalHandler, SIGUSR1);

  l1->run(UV_RUN_DEFAULT);
}

void t2Worker(void* userp) {
  auto l2 = std::make_unique<uvp::LoopObject>();
  auto l3 = std::make_unique<uvp::LoopObject>();
  uvp::SignalObject sig2(l2.get());
  sig2.start(signalHandler, SIGUSR1);
  uvp::SignalObject sig3(l3.get());
  sig3.start(signalHandler, SIGUSR1);

  while (l2->run(UV_RUN_NOWAIT) || l3->run(UV_RUN_NOWAIT));
}

int main(int argc, char *ls[]) {
  uvp::initialize("./", "signal", 1, 3);
  std::cout << "Pid = " << uvp::getpid() << std::endl;

  uvp::Thread t1, t2;
  t1.create(t1Worker, 0);
  t2.create(t2Worker, 0);

  t1.join();
  t2.join();

  LOG_INFO << "quit with return code: " << 0;
  return 0;
}
