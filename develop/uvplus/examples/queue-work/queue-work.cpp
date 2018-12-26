#include <iostream>
#include <memory>

#include <uvp.hpp>

#define FIB_UNTIL 25

long fib_(long t) {
  if (t == 0 || t == 1)
    return 1;
  else
    return fib_(t - 1) + fib_(t - 2);
}

void fib(uvp::Work *req) {
  int n = ((std::pair<int, long> *)req->data())->first;
  if (random() % 2)
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  else
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
  long fib = fib_(n);
  ((std::pair<int, long> *)req->data())->second = fib;
  // std::cerr << n << "th fibonacci is " << fib << std::endl;
}

void afterFib(uvp::Work *req, int status) {
  std::cerr << "Done calculating " << *(int *)req->data()
            << "th fibonacci, the fib is "
            << ((std::pair<int, long> *)req->data())->second << std::endl;
}

int main(int argc, char *ls[]) {
  uvp::initialize("./", "queue-work", 1, 3);

  auto l = std::make_unique<uvp::LoopObject>();
  std::pair<int, long> data[FIB_UNTIL];
  uvp::WorkReq req[FIB_UNTIL];

  for (int i = 0; i < FIB_UNTIL; ++i) {
    data[i].first = i;
    req[i].data((void *)&data[i]);
    l->queueWork(&req[i], fib, afterFib);
  }

  l->run(UV_RUN_DEFAULT);
  l->close();

  LOG_INFO << "quit with return code: " << 0;
  return 0;
}
