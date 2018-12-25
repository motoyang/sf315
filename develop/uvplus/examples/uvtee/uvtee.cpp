#include <iostream>
#include <memory>

#include <uvp.hpp>

void writeData(uvp::Stream *dest, size_t size, const uvp::uv::BufT *buf) {
  auto onWrite = [](uvp::Stream *, int status, uvp::uv::BufT bufs[],
                    int nbufs) {
    for (int i = 0; i < nbufs; ++i) {
      uvp::freeBuf(bufs[i]);
    }
  };

  uvp::uv::BufT n_buf = uvp::allocBuf(size);
  memcpy(n_buf.base, buf->base, size);

  dest->write(&n_buf, 1, onWrite);
}

int main(int argc, char *argv[]) {
  uvp::initialize("./", "uvtee", 1, 3);
  auto l = std::make_unique<uvp::LoopObject>();

  uvp::PipeObject stdinPipe(l.get(), 0);
  stdinPipe.open(0);
  uvp::PipeObject stdoutPipe(l.get(), 0);
  stdoutPipe.open(1);
  uvp::FsReq req;
  int fd = l->fsOpen(&req, argv[1], O_CREAT | O_RDWR | O_TRUNC, 0644, nullptr);
  uvp::PipeObject filePipe(l.get(), 0);
  filePipe.open(fd);

  auto readStdin = [&](uvp::Stream *, ssize_t nread, const uvp::uv::BufT *buf) {
    if (nread < 0) {
      if (nread == UV_EOF) {
        stdinPipe.close(nullptr);
        stdoutPipe.close(nullptr);
        filePipe.close(nullptr);
      }
    } else if (nread > 0) {
      writeData(&stdoutPipe, nread, buf);
      writeData(&filePipe, nread, buf);
    }
  };

  stdinPipe.readStart(nullptr, readStdin);
  l->run(UV_RUN_DEFAULT);
  l->close();

  LOG_INFO << "quit with return code: " << 0;
  return 0;
}
