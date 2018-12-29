#include <iostream>
#include <inttypes.h>

#include <uvp.hpp>

void invokeCgiScript(uvp::Stream* server, uvp::Tcp *client, uvp::Process* child) {
  size_t size = 500;
  char path[size];
  uvp::exepath(path, &size);
  strcpy(path + (strlen(path) - strlen("cgi")), "tick");

  auto cleanupHandles = [server, client](uvp::Process *process, int64_t exit_status,
                                  int term_signal) {
    std::cerr << "Process exited with status " << exit_status << ", signal "
              << term_signal << std::endl;
    // auto client = (uvp::Tcp*)process->data();
    process->close(nullptr);
    client->close(nullptr);
    server->close(nullptr);
  };

  char *args[2];
  args[0] = path;
  args[1] = NULL;

  uvp::Process::Options options = {0};
  options.stdio_count = 3;
  uvp::Process::StdioContainer child_stdio[3];
  child_stdio[0].flags = UV_IGNORE;
  child_stdio[1].flags = UV_INHERIT_STREAM;
  child_stdio[1].data.stream = client->stream();
  child_stdio[2].data.fd = 2;
  options.stdio = child_stdio;

  options.exit_cb = cleanupHandles;
  options.file = args[0];
  options.args = args;

  child->data(client);
  uvp::Loop *l = client->loop();
  int r = child->spawn(l, &options);
  if (r) {
    std::cerr << "spawn error: " << uvp::Error(r).strerror() << std::endl;
    return;
  } else {
    std::cout << "launched tick with PID " << child->getPid() << std::endl;
  }
}

int main(int argc, char *ls[]) {
  uvp::initialize("./", "cgi", 1, 3);
  auto l = std::make_unique<uvp::LoopObject>();

  uvp::ProcessObject child;
  uvp::TcpObject server(l.get()), client(l.get());
  auto onNewConnection = [&client, &child](uvp::Stream *server, int status) {
    if (status == -1) {
      return;
    }
    if (server->accept(&client) == 0) {
      invokeCgiScript(server, &client, &child);
    } else {
      client.close(nullptr);
    }
  };

  sockaddr_in bindAddr = {0};
  uvp::ip4Addr("0.0.0.0", 7000, &bindAddr);
  server.bind((const sockaddr *)&bindAddr, 0);
  int r = server.listen(128, onNewConnection);
  if (r) {
    std::cerr << "Listen error: " << uvp::Error(r).strerror() << std::endl;
    return r;
  }

  l->run(UV_RUN_DEFAULT);
  l->close();

  LOG_INFO << "quit with return code: " << 0;
  return 0;
}