#include <iostream>

#include <uvp.hpp>

void onExit(uvp::Process *handle, int64_t exit_status, int term_signal) {
  std::cerr << "Process exited with status " << exit_status << ", signal "
            << term_signal << std::endl;
  handle->close(nullptr);
}

int main(int argc, char *ls[]) {
  uvp::initialize("./", "spawn", 1, 3);
  auto l = std::make_unique<uvp::LoopObject>();
  char *args[3];
  args[0] = (char*)"mkdir";
  args[1] = (char*)"test-dir";
  args[2] = nullptr;
  uvp::Process::Options options = {0};
  options.exit_cb = onExit;
  options.file = "mkdir";
  options.args = args;

  uvp::ProcessObject child;
  int r = child.spawn(l.get(), &options);
  if (r) {
    std::cerr << "spawn error: " << uvp::Error(r).strerror() << std::endl;
    return r;
  } else {
    std::cout << "launched process with ID " << child.getPid() << std::endl;
  }

  l->run(UV_RUN_DEFAULT);
  l->close();

  LOG_INFO << "quit with return code: " << 0;
  return 0;
}
