// #include "s5acceptor.h"
#include "acceptor.h"
#include "server.h"

// --

int tcp_server() {
  sockaddr_in addr;
  uvp::ip4Addr("0", 7002, &addr);

  auto loop = std::make_unique<uvp::LoopObject>();
  Acceptor server(loop.get(), (const sockaddr *)&addr);
  // S5Acceptor server(loop.get(), (const sockaddr *)&addr);


  int r = loop->run(UV_RUN_DEFAULT);
  UVP_LOG_ERROR(r);
  r = loop->close();
  UVP_LOG_ERROR(r);

  LOG_INFO << "net job closed.";
  return r;
}