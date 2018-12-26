#include <iostream>
#include <inttypes.h>

#include <uvp.hpp>

void onExit(uvp::Process *handle, int64_t exit_status, int term_signal) {
  std::cerr << "Process exited with status " << exit_status << ", signal "
            << term_signal << std::endl;
  handle->close(nullptr);
}

int main(int argc, char *ls[]) {
  uvp::initialize("./", "proc-streams", 1, 3);
  auto l = std::make_unique<uvp::LoopObject>();

  size_t size = 500;
  char path[size];
  uvp::exepath(path, &size);
  strcpy(path + (strlen(path) - strlen("proc-streams")), "proc-test");

  char *args[2];
  args[0] = path;
  args[1] = NULL;

  uvp::Process::Options options;
  options.stdio_count = 3;
  uvp::Process::StdioContainer child_stdio[3];
  child_stdio[0].flags = UV_IGNORE;
  child_stdio[1].flags = UV_IGNORE;
  child_stdio[2].flags = UV_INHERIT_FD;
  child_stdio[2].data.fd = 2;
  options.stdio = child_stdio;

  options.exit_cb = onExit;
  options.file = args[0];
  options.args = args;

  uvp::ProcessObject child;
  int r = child.spawn(l.get(), &options);
  if (r) {
    std::cerr << "spawn error: " << uvp::Error(r).strerror() << std::endl;
    return r;
  } else {
    std::cout << "launched proc-test with PID " << child.getPid() << std::endl;
  }
  child.close(nullptr);
  child.unref();

  l->run(UV_RUN_DEFAULT);
  l->close();

  LOG_INFO << "quit with return code: " << 0;
  return 0;
}