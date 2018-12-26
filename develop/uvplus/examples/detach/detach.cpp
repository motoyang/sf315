#include <iostream>

#include <uvp.hpp>

int main(int argc, char *ls[]) {
  uvp::initialize("./", "detach", 1, 3);
  auto l = std::make_unique<uvp::LoopObject>();
  char *args[3];
  args[0] = (char*)"sleep";
  args[1] = (char*)"100";
  args[2] = nullptr;
  uvp::Process::Options options = {0};
  options.exit_cb = nullptr;
  options.file = "sleep";
  options.args = args;
  options.flags = UV_PROCESS_DETACHED;

  uvp::ProcessObject child;
  int r = child.spawn(l.get(), &options);
  if (r) {
    std::cerr << "spawn error: " << uvp::Error(r).strerror() << std::endl;
    return r;
  } else {
    std::cout << "launched sleep with PID " << child.getPid() << std::endl;
  }
  child.close(nullptr);
  child.unref();

  l->run(UV_RUN_DEFAULT);
  l->close();

  LOG_INFO << "quit with return code: " << 0;
  return 0;
}
