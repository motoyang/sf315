#include <iostream>
#include <memory>

#include <uvp.hpp>

int main(int argc, char *argv[]) {
  uvp::initialize("./", "tty", 1, 3);
  auto l = std::make_unique<uvp::LoopObject>();
  uvp::TtyObject tty(l.get(), 1, 0);
  tty.setMode(UV_TTY_MODE_NORMAL);

  if (uvp::guessHandle(1) == UV_TTY) {
    uvp::uv::BufT buf = {0};
    buf.base = (char*)"\033[41;37m";
    buf.len = strlen(buf.base);
    tty.write(&buf, 1, nullptr);
  }

  uvp::uv::BufT buf = {0};
  buf.base = (char*)"Hello TTY\n";
  buf.len = strlen(buf.base);
  tty.write(&buf, 1, nullptr);
  tty.resetMode();

  l->run(UV_RUN_DEFAULT);
  l->close();

  LOG_INFO << "quit with return code: " << 0;
  return 0;
}
