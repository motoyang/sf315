#include <uvp.hpp>
#include <uvplus.hpp>

#include "secureacceptor.h"
#include "business.h"
#include "server.h"

// --

SecureAcceptor *g_acceptor;
Business *g_business;

static int uvloopRun(uvp::Loop *loop) {
  int r = loop->run(UV_RUN_DEFAULT);
  UVP_LOG_ERROR(r);
  r = loop->close();
  UVP_LOG_ERROR(r);

  LOG_INFO << "net job closed.";
  return r;
}

int tcp_server() {
  // 建立线程池，线程个数1
  // uvplus::ThreadPool pool(1);

  auto loop = std::make_unique<uvp::LoopObject>();
  sockaddr_in addr;
  uvp::ip4Addr("0", 7001, &addr);
  SecureAcceptor acceptor(loop.get(), (const sockaddr *)&addr);
  // S5Acceptor acceptor(loop.get(), (const sockaddr *)&addr);

  g_acceptor = &acceptor;
  Business bness("business");
  bness.bind(&acceptor);

  g_business = &bness;

  std::thread t1(uvloopRun, loop.get());
  bness();

  t1.join();

  // pool.stop();
  // pool.join_all();

  return 0;
}