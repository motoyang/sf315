#include <iostream>
#include <memory>
#include <unordered_map>

#include <uvp.hpp>

int main(int argc, char *argv[]) {
  uvp::initialize("./", "pipe-echo-server", 1, 3);
  auto l = std::make_unique<uvp::LoopObject>();

  std::unordered_map<void *, std::unique_ptr<uvp::PipeObject>> clients;

  auto echoShutdown = [&clients](uvp::Stream *client, int status) {
    if (status < 0) {
      std::cerr << "shutdown error: " << uvp::Error(status).strerror()
                << std::endl;
    }
    client->close(nullptr);
    clients.erase(client);
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
      client->shutdown(echoShutdown);
    } else if (nread < 0) {
      if (nread != UV_EOF) {
        std::cerr << "read error: " << uvp::Error(nread).strerror()
                  << std::endl;
      }
      client->close(nullptr);
    }
  };

  auto onNewConnection = [&l, &clients, &echoRead](uvp::Stream *server,
                                                int status) {
    if (status < 0) {
      std::cerr << "new connection error: " << uvp::Error(status).strerror()
                << std::endl;
      return;
    }

    auto client = std::make_unique<uvp::PipeObject>(l.get(), 0);
    int r = server->accept(client.get());
    if (0 == r) {
      client->readStart(nullptr, echoRead);
      clients.insert({client.get(), std::move(client)});
    } else {
      client->close(nullptr);
    }
  };

  uvp::PipeObject server(l.get(), 0);
  int r = server.bind("echo.sock");
  if (r) {
    std::cerr << "bind error: " << uvp::Error(r).strerror() << std::endl;
    return r;
  }
  r = server.listen(128, onNewConnection);
  if (r) {
    std::cerr << "listen error: " << uvp::Error(r).strerror() << std::endl;
    return r;  
  }

  auto signalHandler = [&](uvp::Signal *sig, int signum) {
    for (auto& c: clients) {
      c.second->shutdown(echoShutdown);
    }
    server.close(nullptr);

    sig->stop();
    sig->close(nullptr);
  };
  uvp::SignalObject sig(l.get());
  sig.start(signalHandler, SIGINT);

  l->run(UV_RUN_DEFAULT);
  l->close();

  LOG_INFO << "quit with return code: " << r;
  return r;
}
