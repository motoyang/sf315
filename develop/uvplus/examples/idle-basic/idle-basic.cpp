#include <iostream>
#include <memory>

#include <uvp.hpp>

int main(int argc, char *argv[]) {
  int64_t counter = 0;
  uvp::initialize("./", "idle-basic", 1, 3);
  
  auto l = std::make_unique<uvp::LoopObject>();
  uvp::IdleObject idler(l.get());
  idler.start([&counter, &idler]() {
    counter++;
    if (counter >= 10e6) {
      idler.stop();
      idler.close(nullptr);
      // idler.close([](){
      //   std::cout << "handle of idle closed." << std::endl;
      // });
    }
  });
  l->run(UV_RUN_DEFAULT);
  l->close();

  return 0;
}
