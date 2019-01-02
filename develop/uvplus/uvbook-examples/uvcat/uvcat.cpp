#include <iostream>
#include <memory>

#include <uvp.hpp>

int main(int argc, char *argv[]) {
  uvp::initialize("./", "uvcat", 1, 3);

  char buffer[1024] = {0};
  uvp::uv::BufT iov = uvp::initBuf(buffer, sizeof(buffer));
  uvp::FsReq readReq, writeReq, openReq;
  auto l = std::make_unique<uvp::LoopObject>();

  uvp::Fs::Callback onRead;
  auto onWrite = [&](uvp::Fs *req) {
    ssize_t r = req->getResult();
    if ( r < 0) {
      std::cout << "write error: " << uvp::Error(r).strerror()
                << std::endl;
    } else {
      l->fsRead(&readReq, openReq.getResult(), &iov, 1, -1, onRead);
    }
  };

  onRead = [&](uvp::Fs *req) {
    ssize_t r = req->getResult();
    if (r < 0) {
      std::cout << "read error: " << uvp::Error(r).strerror()
                << std::endl;
    } else if (r == 0) {
      uvp::FsReq closeReq;
      l->fsClose(&closeReq, openReq.getResult(), nullptr);
    } else {
      iov.len = r;
      l->fsWrite(&writeReq, 1, &iov, 1, -1, onWrite);
    }
  };

  auto onOpen = [&](uvp::Fs *req) {
    ssize_t r = req->getResult();
    if (r > 0) {
      l->fsRead(&readReq, req->getResult(), &iov, 1, -1, onRead);
    } else {
      std::cout << "error openning file: " << uvp::Error(r).strerror()
                << std::endl;
    }
  };

  l->fsOpen(&openReq, argv[1], O_RDONLY, 0, onOpen);
  l->run(UV_RUN_DEFAULT);
  l->close();

  LOG_INFO << "quit with return code: " << 0;
  return 0;
}
