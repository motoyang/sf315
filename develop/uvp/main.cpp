#include <stdio.h>
#include <uv.h>

#include <iostream>

#include <nanolog/nanolog.hpp>

#include <misc.hpp>
#include <uv.hpp>

// --

void test_idle(LoopT *loop) {
  int counter = 0;
  IdleT idler(loop);
  idler.start([&idler, &counter]() {
    counter++;
    if (counter >= 10e6) {
      idler.stop();
      idler.close([]() { std::cout << "idle closed." << std::endl; });
    }
  });
  idler.data(&counter);

  loop->walk(
      [](HandleI *h, void *arg) {
        std::cout << "walk, handle is " << HandleI::typeName(h->type())
                  << std::endl;
      },
      0);

  std::cout << "Idling..." << std::endl;
  loop->run(UV_RUN_DEFAULT);

  std::cout << "counter: " << *(int *)idler.data() << std::endl;
  std::cout << "idler.isActive(): " << idler.isActive() << std::endl;
  std::cout << "idler.isClosing(): " << idler.isClosing() << std::endl;
  std::cout << "idler.hasRef(): " << idler.hasRef() << std::endl;
  std::cout << "ldler.type(): " << IdleT::typeName(idler.type()) << std::endl;
}

// --

void test_timer(LoopT *loop) {
  int counter = 0;
  TimerT timer(loop);
  auto cb = [&timer, &counter]() {
    ++counter;
    if (counter > 10) {
      timer.stop();
      if (counter == 11) {
        std::cout << "repeat is " << timer.repeat()
                  << ". set repeat to 1000 and again. now is "
                  << timer.loop()->now() << std::endl;
        timer.repeat(1000);
        timer.again();
      } else {
        timer.close([&timer]() {
          std::cout << "timer closed. repeat is " << timer.repeat()
                    << ". now is " << timer.loop()->now() << std::endl;
        });
      }
    }
  };
  timer.timerCallback(cb);
  timer.start(1000, 200);
  timer.data(&counter);

  loop->walk(
      [](HandleI *h, void *arg) {
        std::cout << "walk, handle is " << HandleI::typeName(h->type())
                  << ". arg is " << arg << "." << std::endl;
      },
      0);

  std::cout << "Timer..." << std::endl;
  loop->run(UV_RUN_DEFAULT);

  std::cout << "counter: " << *(int *)timer.data() << std::endl;
  std::cout << "timer.isActive(): " << timer.isActive() << std::endl;
  std::cout << "timer.isClosing(): " << timer.isClosing() << std::endl;
  std::cout << "timer.hasRef(): " << timer.hasRef() << std::endl;
  std::cout << "ldler.type(): " << IdleT::typeName(timer.type()) << std::endl;
}

void test_tcp_client(LoopT *loop) {}

// --

static char buffer[1024];

static uv_buf_t iov;

uv_fs_t open_req;
uv_fs_t read_req;
uv_fs_t write_req;

void on_write(uv_fs_t *req);

void on_read(uv_fs_t *req) {
  if (req->result < 0) {
    fprintf(stderr, "Read error: %s\n", uv_strerror(req->result));
  } else if (req->result == 0) {
    uv_fs_t close_req;
    // synchronous
    uv_fs_close(uv_default_loop(), &close_req, open_req.result, NULL);
  } else if (req->result > 0) {
    iov.len = req->result;
    uv_fs_write(uv_default_loop(), &write_req, 1, &iov, 1, -1, on_write);
  }
}

void on_write(uv_fs_t *req) {
  if (req->result < 0) {
    fprintf(stderr, "Write error: %s\n", uv_strerror((int)req->result));
  } else {
    uv_fs_read(uv_default_loop(), &read_req, open_req.result, &iov, 1, -1,
               on_read);
  }
}

void on_open(uv_fs_t *req) {
  // The request passed to the callback is the same as the one the call setup
  // function was passed.
  assert(req == &open_req);
  if (req->result >= 0) {
    iov = uv_buf_init(buffer, sizeof(buffer));
    uv_fs_read(uv_default_loop(), &read_req, req->result, &iov, 1, -1, on_read);
  } else {
    fprintf(stderr, "error opening file: %s\n", uv_strerror((int)req->result));
  }
}

void test_cat(LoopT *loop, const char *filename) {
  uv_fs_open(loop->get(), &open_req, filename, O_RDONLY, 0, on_open);
  loop->run(UV_RUN_DEFAULT);

  uv_fs_req_cleanup(&open_req);
  uv_fs_req_cleanup(&read_req);
  uv_fs_req_cleanup(&write_req);
}

// --

void write_data(StreamI *dest, size_t size, const BufT *buf) {
  BufT n_buf = uv_buf_init((char *)malloc(size), size);
  memcpy(n_buf.base, buf->base, size);

  dest->write(&n_buf, 1);
}

void test_tee(LoopT *loop, const char *filename) {
  PipeT stdin_pipe(loop, 0);
  stdin_pipe.open(0);

  PipeT stdout_pipe(loop, 0);
  stdout_pipe.open(1);

  uv_fs_t file_req;
  int fd = uv_fs_open(loop->get(), &file_req, filename,
                      O_CREAT | O_RDWR | O_TRUNC, 0644, NULL);
  PipeT file_pipe(loop, 0);
  file_pipe.open(fd);

  stdin_pipe.readCallback([&stdin_pipe, &stdout_pipe,
                           &file_pipe](ssize_t nread, const BufT *buf) {
    if (nread < 0) {
      if (nread == UV_EOF) {
        // end of file
        stdin_pipe.close([]() { std::cout << "stdin closed." << std::endl; });
        stdout_pipe.close([]() { std::cout << "stdout closed." << std::endl; });
        file_pipe.close([]() { std::cout << "file closed." << std::endl; });
      }
    } else if (nread > 0) {
      write_data(&stdout_pipe, nread, buf);
      write_data(&file_pipe, nread, buf);
    }
  });
  auto cb = [](int status, BufT *bufs, int nbufs) {
    for (int i = 0; i < nbufs; ++i) {
      char *p = (bufs + i)->base;
      free(p);
    }
  };
  stdout_pipe.writeCallback(cb);
  file_pipe.writeCallback(cb);

  stdin_pipe.readStart();
  loop->run(UV_RUN_DEFAULT);
}

// --

int main(int argc, char *argv[]) {
  const unsigned int log_file_count = 5;
  nanolog::initialize(nanolog::GuaranteedLogger(), "/tmp/", "uvp", 1,
                      log_file_count);

  std::cout << "libuv version: " << Version().str() << std::endl;
  auto loop = LoopT::defaultLoop();
  // auto loop = std::make_unique<LoopT>();

  // test_tee(loop.get(), argv[1]);
  // test_cat(loop.get(), argv[1]);
  // test_idle(loop.get());
  test_timer(loop.get());

  loop->close();
  return 0;
}
