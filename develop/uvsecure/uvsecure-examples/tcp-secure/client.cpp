#include <iostream>
#include <cstdlib>

#include <uvp.hpp>
#include <uvplus.hpp>
#include <botan/hex.h>

#include <pp/prettyprint.h>

#include "secureconnector.h"
#include "client.h"

// --

int netStart(uvp::Loop *loop) {
  int r = loop->run(UV_RUN_DEFAULT);
  UVP_LOG_ERROR(r);
  r = loop->close();
  UVP_LOG_ERROR(r);

  LOG_INFO << "net job closed.";
  return r;
}

bool g_running = true;

int f_output(SecureConnector *client) {
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  auto j = 0;
  for (int i = 0; i < 1500000; ++i) {
    u8vector buf(512 + (i % 13000), 0);
    for (auto &c : buf) {
      c = j++;
    }
    client->write(buf.data(), buf.size());
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(20000));
  g_running = false;
  return 0;
}

int tcp_client() {
  sockaddr_in dest;
  uvp::ip4Addr("127.0.0.1", 7001, &dest);

  auto loop = std::make_unique<uvp::LoopObject>();
  // uvplus::TcpConnector client(loop.get(), (const sockaddr *)&dest);
  SecureConnector client(loop.get(), (const sockaddr *)&dest);
  std::thread t1(netStart, loop.get());
  std::thread t2(f_output, &client);

  while (g_running) {
    // u8vector bufs[10];
    u8vlist bufs;
    bufs.resize(10);
    auto count = client.read(bufs);
    for (int i = 0; i < count; ++i) {
      auto v(bufs.front());
      bufs.pop_front();
      auto s = Botan::hex_encode(v);
      std::cout << s << std::endl;
    }
  }

  client.notify(1);
  t1.join();
  t2.join();

  return 0;
}
