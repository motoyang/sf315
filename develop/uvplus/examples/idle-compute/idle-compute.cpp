#include <iostream>
#include <memory>

#include <uvp.hpp>

int main(int argc, char *argv[]) {
  uvp::initialize("./", "idle-compute", 1, 3);
  
  char buffer[1024] = {0};
  uvp::uv::BufT buf = uvp::initBuf(buffer, sizeof(buffer));
  uvp::FsReq stdin_watcher;

  auto l = std::make_unique<uvp::LoopObject>();
  uvp::IdleObject idler(l.get());

  auto ic = [&idler]() {
    std::cout << "Computing PI..." << std::endl;
    idler.stop();
  };

  uvp::Fs::Callback onType;
  auto ot = [&](uvp::uv::FsT *req) {
    if (req->result > 0) {
      buffer[req->result] = 0;
      std::cout << "Typed: " << buffer << std::endl;

      l->fsRead(&stdin_watcher, 0, &buf, 1, -1, onType);
      idler.start(ic);
    } else if (req->result < 0) {
      std::cout << "error opening file: " << uvp::Error(req->result).strerror()
                << std::endl;
    } else {
      std::cout << "end of file." << std::endl;
      idler.close(nullptr);
    }
  };
  onType = ot;

  l->fsRead(&stdin_watcher, 0, &buf, 1, -1, onType);
  idler.start(ic);

  l->run(UV_RUN_DEFAULT);
  l->close();

  return 0;
}
