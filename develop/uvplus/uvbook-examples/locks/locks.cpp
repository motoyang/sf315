#include <iostream>
#include <memory>

#include <uvp.hpp>

int main(int argc, char *argv[]) {
  uvp::initialize("./", "thread-create", 1, 3);

  int shared_num = 0;
  uvp::Barrier blocker(4);
  uvp::Rwlock numlock;
  uvp::Thread threads[3];
  int thread_nums[3] = {1, 2, 3};

  auto reader = [&](void *n) {
    int num = *(int *)n;
    for (int i = 0; i < 20; ++i) {
      numlock.rdlock();
      std::cout << "Reader " << num << " acquired lock." << std::endl;
      std::cout << "Reader " << num << " shared num = " << shared_num
                << std::endl;
      numlock.rdunlock();
      std::cout << "Reader " << num << " released lock." << std::endl;
    }
    blocker.wait();
  };

  auto writer = [&](void* n) {
    int num = *(int*)n;
    for (int i = 0; i < 20; ++i) {
      numlock.wrlock();
      std::cout << "Writer " << num << " acquired lock." << std::endl;
      shared_num++;
      std::cout << "Writer " << num << " incremented shared num = " << shared_num
                << std::endl;
      numlock.wrunlock();
      std::cout << "Writer " << num << " released lock." << std::endl;
    }
    blocker.wait();
  };

  threads[0].create(reader, &thread_nums[0]);
  threads[1].create(reader, &thread_nums[1]);
  threads[2].create(writer, &thread_nums[2]);

  blocker.wait();

  LOG_INFO << "quit with return code: " << 0;
  return 0;
}
