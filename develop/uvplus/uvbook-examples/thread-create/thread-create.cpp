#include <iostream>
#include <memory>

#include <uvp.hpp>

void hare(void *arg) {
  int tracklen = *((int *)arg);
  while (tracklen) {
    tracklen--;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::cerr << "Hare ran another step" << std::endl;
  }
  std::cerr << "Hare done running!" << std::endl;
}

int main(int argc, char *argv[]) {
  uvp::initialize("./", "thread-create", 1, 3);
  int tracklen = 10;

  uvp::Thread hare_id;
  hare_id.create(hare, &tracklen);

  uvp::Thread tortoise_id;
  tortoise_id.create(
      [](void *arg) {
        int tracklen = *((int *)arg);
        while (tracklen) {
          tracklen--;
          std::cerr << "Tortoise ran another step" << std::endl;
          std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        }
        std::cerr << "Tortoise done running!" << std::endl;
      },
      &tracklen);

  hare_id.join();
  tortoise_id.join();

  LOG_INFO << "quit with return code: " << 0;
  return 0;
}
