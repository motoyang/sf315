//
// node.h
//

#pragma once

// --

namespace rpcpp2 {

class Node {
protected:
  nng_socket _sock;
  bool _running = true;

public:
  bool run() const {
    return _running;
  }
  int close() {
    _running = false;
    return nng_close(_sock);
  }
  const char* strerror(int e) {
    return nng_strerror(e);
  }
};

}
