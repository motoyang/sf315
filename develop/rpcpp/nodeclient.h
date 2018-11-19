//
// nodeclient.h
//

#pragma once

// --

namespace rpcpp2 {
namespace client {

// --

class SubscribeNode: public Node {
public:
  SubscribeNode(const std::string& url);
  int recv(char **buf, size_t *len);
};

// --

class RequestNode: public Node {
public:
  RequestNode(const std::string& url);
  int send(void* buf, size_t len);
  int recv(char **buf, size_t *len);
};

// --

}
}
