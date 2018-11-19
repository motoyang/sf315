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
    LOG_INFO << "node will be closed.";
    _running = false;
    return nng_close(_sock);
  }
  const char* strerror(int e) {
    return nng_strerror(e);
  }
};

// --

class PushNode: public Node {
public:
  PushNode(const std::string& url);
  int send(void *buf, size_t len);
};

// --

class PullNode: public Node {
public:
  PullNode(const std::string& url);
  int recv(char **buf, size_t *len);
};

// --

}
