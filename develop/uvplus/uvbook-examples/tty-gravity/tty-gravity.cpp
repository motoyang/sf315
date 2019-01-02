#include <iostream>
#include <memory>

#include <uvp.hpp>

int main(int argc, char *argv[]) {
  uvp::initialize("./", "tty-gravity", 1, 3);
  auto l = std::make_unique<uvp::LoopObject>();
  uvp::TtyObject tty(l.get(), 1, 0);
  tty.setMode(UV_TTY_MODE_NORMAL);

  int width = 0, height = 0;
  if (tty.getWinsize(&width, &height)) {
    std::cerr << "Could not get TTY information" << std::endl;
    tty.resetMode();
    return 1;
  }
  std::cout << "width: " << width << ", height: " << height << std::endl;

  const char *message = "  Hello TTY  ";
  int pos = 0;
  auto update = [&tty, &message, &pos, &width, &height](uvp::Timer *timer) {
    char data[500] = {0};
    uvp::uv::BufT buf = uvp::initBuf(
        data, sprintf(data, "\033[2J\033[H\033[%dB\033[%luC\033[42;37m%s", pos,
                      (unsigned long)(width - strlen(message)) / 2, message));
    tty.write(&buf, 1, nullptr);

    if (++pos > height) {
      tty.resetMode();
      buf.base = (char*)"\n";
      buf.len = 1;
      tty.write(&buf, 1, nullptr);
      
      timer->stop();
    }
  };
  uvp::TimerObject tick(l.get());
  tick.start(update, 200, 200);

  l->run(UV_RUN_DEFAULT);
  l->close();

  LOG_INFO << "quit with return code: " << 0;
  return 0;
}
