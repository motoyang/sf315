#include <iostream>
#include <memory>
#include <unordered_map>

#include <uvp.hpp>

struct Workers {
  std::unordered_map<int, std::pair<std::unique_ptr<uvp::ProcessObject>,
                                    std::unique_ptr<uvp::PipeObject>>>
      _workers;
  std::unordered_map<void *, std::unique_ptr<uvp::TcpObject>> _clients;
  uvp::Loop *_loop;
  int _round_robin_counter = 0;
  int _cpuCount = 0;

  Workers(uvp::Loop *l) : _loop(l) {}

  void close() {
    for (auto &p : _clients) {
      p.second->shutdown([this](uvp::Stream *client, int status) {
        if (status < 0) {
          std::cerr << "shutdown error: " << uvp::Error(status).strerror()
                    << std::endl;
        }
        client->close(nullptr);
        _clients.erase(client);
      });
    }
  }

  void closeProcessHandle(uvp::Process *handle, int64_t exit_status,
                          int term_signal) {
    std::cerr << "Process exited with status " << exit_status << ", signal "
              << term_signal << std::endl;
    handle->close(nullptr);
  }

  void onNewConnection(uvp::Stream *server, int status) {
    if (status < 0) {
      std::cerr << "new connection error: " << uvp::Error(status).strerror()
                << std::endl;
      return;
    }

    auto client = std::make_unique<uvp::TcpObject>(_loop);
    int r = server->accept(client.get());
    if (0 == r) {
      uvp::uv::BufT buf = uvp::initBuf((char *)"a", 1);
      uvp::Pipe *pipe = _workers[_round_robin_counter++ % _cpuCount].second.get();
      pipe->write2(&buf, 1, client.get(), nullptr);
    } else {
      client->close(nullptr);
    }
  }

  void setup() {
    char workerPath[500] = {0};
    size_t pathSize = sizeof(workerPath);
    uvp::exepath(workerPath, &pathSize);
    strcpy(workerPath + (strlen(workerPath) - strlen("multi-echo-server")),
           "multi-echo-worker");
    std::cerr << "Worker path: " << workerPath << std::endl;

    char *args[2];
    args[0] = workerPath;
    args[1] = NULL;

    // launch same number of workers as number of CPUs
    uvp::uv::CpuInfoT *info;
    uvp::cpuInfo(&info, &_cpuCount);
    uvp::freeCpuInfo(info, _cpuCount);

    for (int i = 0; i < _cpuCount; ++i) {
      auto worker = std::make_unique<uvp::ProcessObject>();
      auto pipe = std::make_unique<uvp::PipeObject>(_loop, 1);

      uvp::Process::StdioContainer child_stdio[3];
      child_stdio[0].flags =
          (uv_stdio_flags)(UV_CREATE_PIPE | UV_READABLE_PIPE);
      child_stdio[0].data.stream = pipe->stream();
      child_stdio[1].flags = UV_IGNORE;
      child_stdio[2].flags = UV_INHERIT_FD;
      child_stdio[2].data.fd = 2;

      uvp::Process::Options options = {0};
      options.stdio = child_stdio;
      options.stdio_count = 3;

      options.exit_cb =
          std::bind(&Workers::closeProcessHandle, this, std::placeholders::_1,
                    std::placeholders::_2, std::placeholders::_3);
      options.file = args[0];
      options.args = args;

      worker->spawn(_loop, &options);
      std::cout << "Started workder " << worker->getPid() << std::endl;

      _workers.insert({i, std::make_pair(std::move(worker), std::move(pipe))});
    }
  }
};

int main(int argc, char *argv[]) {
  uvp::initialize("./", "multi-echo-server", 1, 3);
  auto l = std::make_unique<uvp::LoopObject>();
  Workers workers(l.get());
  workers.setup();

  sockaddr_in bindAddr;
  uvp::ip4Addr("0.0.0.0", 7000, &bindAddr);
  uvp::TcpObject server(l.get());
  int r = server.bind((const sockaddr *)&bindAddr, 0);
  if (r) {
    std::cerr << "bind error: " << uvp::Error(r).strerror() << std::endl;
    return r;
  }
  r = server.listen(128,
                    std::bind(&Workers::onNewConnection, &workers,
                              std::placeholders::_1, std::placeholders::_2));
  if (r) {
    std::cerr << "listen error: " << uvp::Error(r).strerror() << std::endl;
    return r;
  }

  auto signalHandler = [&](uvp::Signal *sig, int signum) {
    workers.close();
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
