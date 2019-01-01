#include <cassert>
#include <iostream>
#include <memory>
#include <unordered_map>

#include <uvp.hpp>

int main(int argc, char *argv[]) {
  uvp::initialize("./", "multi-echo-worker", 1, 3);
  auto l = std::make_unique<uvp::LoopObject>();
  auto client = std::make_unique<uvp::TcpObject>(l.get());
  uvp::PipeObject queue(l.get(), 1);

  auto echoShutdown = [](uvp::Stream *client, int status) {
    if (status < 0) {
      std::cerr << "shutdown error: " << uvp::Error(status).strerror()
                << std::endl;
    }
    client->close(nullptr);
  };

  auto echoWrite = [](uvp::Stream *client, int status, uvp::uv::BufT bufs[],
                      int nbufs) {
    if (status) {
      std::cerr << "write error: " << uvp::Error(status).strerror()
                << std::endl;
    }
    for (int i = 0; i < nbufs; ++i) {
      uvp::freeBuf(bufs[i]);
    }
  };

  auto echoRead = [&echoWrite, &echoShutdown](uvp::Stream *client,
                                              ssize_t nread,
                                              const uvp::uv::BufT *buf) {
    if (nread > 0) {
      uvp::uv::BufT b = uvp::copyToBuf(buf->base, nread);
      client->write(&b, 1, echoWrite);
      // client->shutdown(echoShutdown);
      client->shutdown(nullptr);
    } else if (nread < 0) {
      if (nread != UV_EOF) {
        std::cerr << "read error: " << uvp::Error(nread).strerror()
                  << std::endl;
      }
      client->close(nullptr);
    }
  };

  auto onNewConnection = [&client, &echoRead](uvp::Stream *q, ssize_t nread,
                                         const uvp::uv::BufT *buf) {
    if (nread < 0) {
      std::cerr << "new connection error: " << uvp::Error(nread).strerror()
                << std::endl;
      return;
    }

    uvp::Pipe *pipe = (uvp::Pipe *)q;
    if (!pipe->pendingCount()) {
      std::cerr << "No pending count." << std::endl;
    }

    uvp::Handle::Type pending = pipe->pendingType();
    assert(pending == uvp::Handle::Type::UV_TCP);

    client->reinit();
    if (pipe->accept(client.get()) == 0) {
      uvp::uv::OsFdT fd;
      client->fileno(&fd);
      std::cerr << "Workder " << uvp::getpid() << " accepted fd " << fd
                << std::endl;
      client->readStart(nullptr, echoRead);
    }
  };
  queue.open(0);
  queue.readStart(nullptr, onNewConnection);

  l->run(UV_RUN_DEFAULT);
  int r = l->close();

  LOG_INFO << "quit with return code: " << r;
  return r;
}
