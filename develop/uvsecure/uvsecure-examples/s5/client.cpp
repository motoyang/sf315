#include <iostream>
#include <cstdlib>

#include "readjson.h"
#include "connector.h"
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

int tcp_client() {
  JsonConfig jc;
  readConfig(ConfigFilename, jc);

  // sockaddr_in dest;
  // uvp::ip4Addr("127.0.0.1", 7002, &dest);

  auto loop = std::make_unique<uvp::LoopObject>();
  Connector client(loop.get(), jc);

  int r = loop->run(UV_RUN_DEFAULT);
  UVP_LOG_ERROR(r);
  r = loop->close();
  UVP_LOG_ERROR(r);

  LOG_INFO << "uvloop closed.";
  return r;
}
