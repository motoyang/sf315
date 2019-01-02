#include <iostream>
#include <memory>

#include <uvp.hpp>

int main(int argc, char *argv[]) {
  int64_t counter = 0;
  uvp::initialize("./", "uvstop", 1, 3);
  auto l = std::make_unique<uvp::LoopObject>();
  uvp::IdleObject idler(l.get());
  uvp::PrepareObject prepare(l.get());

  auto idleCallback = [&counter, &l](uvp::Idle* idle) {
    std::cout << "Idle callback." << std::endl;
    counter++;
    if (counter > 5) {
      idle->close(nullptr);
      l->stop();
      std::cout << "loop.stop() called." << std::endl;
    }
  };
  auto prepareCallback = [&counter](uvp::Prepare* prepare) {
    std::cout << "Prepare callback." << std::endl;
     if (counter > 5) {
      prepare->close(nullptr);
     }
  };
  idler.start(idleCallback);
  prepare.start(prepareCallback);

  l->run(UV_RUN_DEFAULT);
  l->close();

  LOG_INFO << "quit with return code: " << 0;
  return 0;
}
