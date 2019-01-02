#include <iostream>
#include <memory>

#include <uvp.hpp>

int main(int argc, char *argv[]) {
  uvp::initialize("./", "ref-timer", 1, 3);
  auto l = std::make_unique<uvp::LoopObject>();
  uvp::TimerObject gcTimer(l.get());
  gcTimer.unref();
  uvp::TimerObject fakeJobTimer(l.get());

  auto gc = [](uvp::Timer* handle) {
    std::cout << "Free unused objects." << std::endl;
  };
  auto fakeJob = [](uvp::Timer* handle) {
    std::cout << "Fake job done." << std::endl;
  };
  gcTimer.start(gc, 0, 2000);
  fakeJobTimer.start(fakeJob, 9000, 0);

  l->run(UV_RUN_DEFAULT);
  l->close();

  LOG_INFO << "quit with return code: " << 0;
  return 0;
}
