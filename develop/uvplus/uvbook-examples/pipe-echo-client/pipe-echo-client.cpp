#include <iostream>
#include <memory>

#include <uvp.hpp>

int main(int argc, char *argv[]) {
  uvp::initialize("./", "pipe-echo-client", 1, 3);
  auto l = std::make_unique<uvp::LoopObject>();

  auto onWrite = [](uvp::Stream *client, int status, uvp::uv::BufT bufs[],
                      int nbufs) {
    if (status) {
      std::cerr << "write error: " << uvp::Error(status).strerror()
                << std::endl;
    }
    for (int i = 0; i < nbufs; ++i) {
      uvp::freeBuf(bufs[i]);
    }
  };

  auto onRead = [](uvp::Stream *client, ssize_t nread,
                   const uvp::uv::BufT *buf) {
    if (nread > 0) {
      std::cout << "echo from server: " << buf->base << std::endl;
    } else if (nread < 0) {
      if (nread != UV_EOF) {
        std::cerr << "read error: " << uvp::Error(nread).strerror()
                  << std::endl;
      }
      client->close(nullptr);
    }
  };

  auto onConnect = [&onRead, &onWrite](uvp::Stream *client, int status) {
    if (status < 0) {
      std::cerr << "connect error: " << uvp::Error(status).strerror()
                << std::endl;
      return;
    }
    client->readStart(nullptr, onRead);

    const char *hello = "hello echo server!";
    uvp::uv::BufT buf = uvp::copyToBuf(hello, std::strlen(hello));
    client->write(&buf, 1, onWrite);
  };

  uvp::PipeObject client(l.get(), 0);
  client.connect("echo.sock", onConnect);

  int r = l->run(UV_RUN_DEFAULT);
  r = l->close();

  LOG_INFO << "quit with return code: " << r;
  return r;
}
