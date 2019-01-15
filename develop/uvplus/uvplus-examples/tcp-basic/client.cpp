#include <iostream>

#include <uvp.hpp>
#include <uvplus.hpp>

#include <pp/prettyprint.h>

#include "client.h"

// --

// --

int netStart(uvp::Loop *loop) {
  int r = loop->run(UV_RUN_DEFAULT);
  UVP_LOG_ERROR(r);
  r = loop->close();
  UVP_LOG_ERROR(r);

  LOG_INFO << "net job closed.";
  return r;
}

void f21_pair(int i, const std::string &s, float f) {
  static size_t count = 0;
  if (++count % 10000)
    return;
  std::cout << "f21_pair: i = " << i << ", s = " << s << ", f = " << f
            << std::endl;
}

void f22_pair(float f, const std::string &s) {
  static size_t count = 0;
  if (++count % 10000)
    return;
  std::cout << "f22_pair: f = " << f << ", s = " << s << std::endl;
}

void f23_pair(const std::string &s) {
  static size_t count = 0;
  if (++count % 10000)
    return;
  std::cout << "f23_pair: s = " << s << std::endl;
}

void f24_pair(const std::tuple<int, std::string, float, double> &t) {
  static size_t count = 0;
  if (++count % 10000)
    return;
  std::cout << "f24_pair: t = " << t << std::endl;
}

bool g_running = true;

int f_output(uvplus::TcpConnector<uvplus::Codec2> *client) {
  std::srand(std::time(nullptr));
  for (int i = 0; i < 10000; ++i) {
    int r = client->transmit(uvp::BufType::BUF_ECHO_TYPE, 21, 38,
                             std::string("string1"), 8.8f);
    UVP_LOG_ERROR(r);

    r = client->transmit(uvp::BufType::BUF_ECHO_TYPE, 22, 0.86,
                         std::string("string2"));
    UVP_LOG_ERROR(r);

    if (i % 10000 == 0) {
      int i2 = std::rand() % 100, j2 = std::rand() % 100;
      int result = 0;
      if (0 == client->request(result, 31, i2, j2)) {
        std::cout << i2 << " + " << j2 << " = " << result << std::endl;
      } else {
        std::cout << "timeout..." << std::endl;
      }
      std::tuple<int, std::string, int> result2;
      if (0 == client->request(result2, 32)) {
        std::cout << "resutl2 = " << result2 << std::endl;
      } else {
        std::cout << "timeout2..." << std::endl;
      }
    }

    r = client->transmit(uvp::BufType::BUF_RESOLVE_TYPE, 23, 99, (double)0.1386,
                         std::string("string3"));
    UVP_LOG_ERROR(r);

    r = client->transmit(
        uvp::BufType::BUF_ECHO_TYPE, 24,
        std::make_tuple(36, std::string("str in tuple"), 1.88f, (double)0.248));
    UVP_LOG_ERROR(r);
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  g_running = false;
  return 0;
}

int f_output2(uvplus::TcpConnector<uvplus::Codec2> *client) {
  for (int i = 0; i < 20000000; ++i) {
    std::string s(std::rand() % 46 + 1, '=');
    client->downwardEnqueue(s.data(), s.length());
  }

  for (;;) {
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
  }
  return 0;
}

int tcp_client() {
  auto resolver = std::make_unique<uvplus::Resolver<int>>();
  resolver->defineFun(21, f21_pair)
      .defineFun(22, f22_pair)
      .defineFun(23, f23_pair)
      .defineFun(24, f24_pair);

  sockaddr_in dest;
  uvp::ip4Addr("65.49.221.234", 7001, &dest);

  auto loop = std::make_unique<uvp::LoopObject>();
  uvplus::TcpConnector<uvplus::Codec2> client(loop.get(),
                                              (const sockaddr *)&dest);
  std::thread t1(netStart, loop.get());
  std::thread t2(f_output, &client);

  while (g_running) {
    uvp::uv::BufT b[10] = {0};
    size_t count = client.upwardDequeue(b, COUNT_OF(b));
    for (int i = 0; i < count; ++i) {
      size_t offset = 0;
      msgpack::object_handle oh = msgpack::unpack(b[i].base, b[i].len, offset);
      int buf_type = 0;
      oh.get().convert(buf_type);
      msgpack::object_handle oh2 = msgpack::unpack(b[i].base, b[i].len, offset);
      int token = 0;
      oh2.get().convert(token);
      resolver->resolve(b[i].base, b[i].len, offset);

      uvp::freeBuf(b[i]);
    }
  }

  client.notify(1);
  t1.join();
  t2.join();

  return 0;
}
