#pragma once

//

namespace rpcpp2 {
namespace server {

// --

class PublishNode: public Node {
public:
  PublishNode(const std::string & url);
  int send(void *buf, size_t len);
};

// --

class ReplyNode: public Node {
public:
  ReplyNode(std::string const& url);
  int recv(char **buf, size_t *len);
  int send(void *buf, size_t len);
};

// --

class PairNode: public Node {
public:
  PairNode(const std::string& url);
  int recv(char **buf, size_t *len);
  int send(void *buf, size_t len);
};

}
}
