#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <uv.h>

uv_loop_t *loop;
uv_pipe_t queue;

typedef struct {
    uv_write_t req;
    uv_buf_t buf;
} write_req_t;

void free_write_req(uv_write_t *req) {
    write_req_t *wr = (write_req_t*) req;
    free(wr->buf.base);
    free(wr);
}

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
  buf->base = (char*)malloc(suggested_size);
  buf->len = suggested_size;
}

void echo_write(uv_write_t *req, int status) {
    if (status) {
        fprintf(stderr, "Write error %s\n", uv_err_name(status));
    }
    free_write_req(req);
}

void echo_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
    if (nread > 0) {
        write_req_t *req = (write_req_t*) malloc(sizeof(write_req_t));
        req->buf = uv_buf_init(buf->base, nread);
        uv_write((uv_write_t*) req, client, &req->buf, 1, echo_write);
        return;
    }

    if (nread < 0) {
        if (nread != UV_EOF)
            fprintf(stderr, "Read error %s\n", uv_err_name(nread));
        uv_close((uv_handle_t*) client, NULL);
    }

    free(buf->base);
}

void on_new_connection(uv_stream_t *q, ssize_t nread, const uv_buf_t *buf) {
    if (nread < 0) {
        if (nread != UV_EOF)
            fprintf(stderr, "Read error %s\n", uv_err_name(nread));
        uv_close((uv_handle_t*) q, NULL);
        return;
    }

    uv_pipe_t *pipe = (uv_pipe_t*) q;
    if (!uv_pipe_pending_count(pipe)) {
        fprintf(stderr, "No pending count\n");
        return;
    }

    uv_handle_type pending = uv_pipe_pending_type(pipe);
    assert(pending == UV_TCP);

    uv_tcp_t *client = (uv_tcp_t*) malloc(sizeof(uv_tcp_t));
    uv_tcp_init(loop, client);
    if (uv_accept(q, (uv_stream_t*) client) == 0) {
        uv_os_fd_t fd;
        uv_fileno((const uv_handle_t*) client, &fd);
        fprintf(stderr, "Worker %d: Accepted fd %d\n", getpid(), fd);
        uv_read_start((uv_stream_t*) client, alloc_buffer, echo_read);
    }
    else {
        uv_close((uv_handle_t*) client, NULL);
    }
}

int main() {
    loop = uv_default_loop();

    uv_pipe_init(loop, &queue, 1 /* ipc */);
    uv_pipe_open(&queue, 0);
    uv_read_start((uv_stream_t*)&queue, alloc_buffer, on_new_connection);
    return uv_run(loop, UV_RUN_DEFAULT);
}
/*
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
*/