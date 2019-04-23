#include <iostream>
#include <cstdlib>

#include <uvp.hpp>
#include <uvplus.hpp>
#include <botan/hex.h>

#include <pp/prettyprint.h>

#include "secureconnector.h"
#include "client.h"

// --

static int uvloopRun(uvp::Loop *loop) {
  int r = loop->run(UV_RUN_DEFAULT);
  UVP_LOG_ERROR(r);
  r = loop->close();
  UVP_LOG_ERROR(r);

  LOG_INFO << "net job closed.";
  return r;
}

bool g_running = true;

static int saySomething(SecureConnector *client) {
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  auto j = 0;
  for (int i = 0; i < 1000000; ++i) {
    u8vector buf(512 + (i % 13000), 0);
    for (auto &c : buf) {
      c = j++;
    }
    client->write(buf.data(), buf.size());
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }
  // client->notify(SecureConnector::NotifyTag::KILL);

  std::this_thread::sleep_for(std::chrono::milliseconds(200000));
  g_running = false;
  return 0;
}

int tcp_client() {
  sockaddr_in dest;
  uvp::ip4Addr("127.0.0.1", 7001, &dest);

  auto loop = std::make_unique<uvp::LoopObject>();
  SecureConnector client(loop.get(), (const sockaddr *)&dest);
  std::thread t1(uvloopRun, loop.get());
  std::thread t2(saySomething, &client);

  while (g_running) {
    u8vlist bufs;
    auto count = client.read(bufs);
    while (count--) {
      auto v(bufs.front());
      bufs.pop_front();
      auto s = Botan::hex_encode(v);
      std::cout << s << std::endl;
    }
  }

  client.notify(SecureConnector::NotifyTag::CLOSE);
  t1.join();
  t2.join();

  return 0;
}
