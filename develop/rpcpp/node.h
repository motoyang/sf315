//
// node.h
//

#pragma once

#include <iostream>
#include <sstream>

#include <msgpack.hpp>
#include <nng/nng.h>

#include "socket.h"
#include "resolver.h"

// --

namespace rpcpp {

// --

template <typename T, typename D> class Send {
public:
  template <typename... Args> int transmit(T const &tag, Args &&... args) {
    int r = -1;
    std::stringstream ss;
    msgpack::pack(ss, tag);
    ((msgpack::pack(ss, std::forward<Args>(args)), ...));

    return ((D *)this)->send(ss.str().data(), ss.str().size());
  }
};

// --

template <typename D> class Recv {
public:
  int transact() {
    char *data = nullptr;
    size_t len = 0, offset = 0;
    int r = ((D *)this)->recv(&data, &len);

    std::string result = ((D *)this)->resolver()->resolve(data, len, offset);
    nng_free(data, len);

    return 0;
  }
};

// --

template <typename D> class Reply {
public:
  int transact() {
    char *data = nullptr;
    size_t len = 0, offset = 0;
    int r = ((D *)this)->recv(&data, &len);

    std::string result = ((D *)this)->replier()->reply(data, len, offset);
    nng_free(data, len);

    if (result.size() > 0) {
      r = ((D *)this)->send(result.data(), result.size());
    }

    return r;
  }
};

template <typename T, typename D> class Request {
public:
  template <typename R, typename... Args>
  int write(R &result, T const &tag, Args &&... args) {
    int r = -1;
    std::stringstream ss;
    msgpack::pack(ss, tag);
    ((msgpack::pack(ss, std::forward<Args>(args)), ...));
    return ((D *)this)->send(ss.str().data(), ss.str().size());
  }

  template <typename R, typename... Args>
  int request(R &result, T const &tag, Args &&... args) {
    int r = -1;
    std::stringstream ss;
    msgpack::pack(ss, tag);
    ((msgpack::pack(ss, std::forward<Args>(args)), ...));
    ((D *)this)->send(ss.str().data(), ss.str().size());

    char *buf = NULL;
    size_t len = 0;
    ((D *)this)->recv(&buf, &len);
    msgpack::object_handle oh = msgpack::unpack(buf, len);
    oh.get().convert(result);

    nng_free(buf, len);

    return 0;
  }

  template <typename R, typename... Args>
  int requestOnObj(R &result, T const &cn, T const &tag, Pointer oid,
                   Args &&... args) {
    int r = -1;
    std::stringstream ss;
    msgpack::pack(ss, cn);
    msgpack::pack(ss, tag);
    msgpack::pack(ss, oid);
    ((msgpack::pack(ss, std::forward<Args>(args)), ...));
    ((D *)this)->send(ss.str().data(), ss.str().size());

    char *buf = NULL;
    size_t len = 0;
    ((D *)this)->recv(&buf, &len);
    msgpack::object_handle oh = msgpack::unpack(buf, len);
    oh.get().convert(result);

    nng_free(buf, len);
    return 0;
  }
};

// --

class Node {
protected:
  Socket _sock;

public:
  Node(Socket::OpenFun f);
  virtual ~Node() {}

  bool isRunning() const;
  int close();
  int dial(const char *url);
  int listen(const char *url);
  int recv(char **buf, size_t *len);
  int send(void *data, size_t len);
  const char *strerror(int e);
};

// --

template <typename T>
class PushNode : public Node, public Send<T, PushNode<T>> {
public:
  PushNode() : Node(OpenAsPush) {}
};

// --

template <typename T>
class PublishNode : public Node, public Send<T, PublishNode<T>> {
public:
  PublishNode() : Node(OpenAsPub) {}
};

// --

template <typename T> class PullNode : public Node, public Recv<PullNode<T>> {
  std::unique_ptr<Resolver<T>> _resolver;

public:
  PullNode(std::unique_ptr<Resolver<T>> &&r)
      : Node(OpenAsPull), _resolver(std::forward<decltype(r)>(r)) {}

  Resolver<T> *resolver() const { return _resolver.get(); }
};

// --

template <typename T>
class SubscribeNode : public Node, public Recv<SubscribeNode<T>> {
  std::unique_ptr<Resolver<T>> _resolver;

public:
  SubscribeNode(std::unique_ptr<Resolver<T>> &&r)
      : Node(OpenAsSub), _resolver(std::forward<decltype(r)>(r)) {}

  Resolver<T> *resolver() const { return _resolver.get(); }
};

// --

template <typename T>
class RequestNode : public Node, public Request<T, RequestNode<T>> {
public:
  RequestNode() : Node(OpenAsReq) {}
};

// --

template <typename T>
class ReplyNode : public Node, public Reply<ReplyNode<T>> {
  std::unique_ptr<Replier<T>> _replier;

public:
  ReplyNode(std::unique_ptr<Replier<T>> &&r)
      : Node(OpenAsRep), _replier(std::forward<decltype(r)>(r)) {}

  Replier<T> *replier() const { return _replier.get(); }
};

// --

template <typename T>
class PairNode : public Node,
                 public Recv<PairNode<T>>,
                 public Send<T, PairNode<T>> {
  std::unique_ptr<Resolver<T>> _resolver;

public:
  PairNode(std::unique_ptr<Resolver<T>> &&r)
      : Node(OpenAsPair), _resolver(std::forward<decltype(r)>(r)) {}

  Resolver<T> *resolver() const { return _resolver; }
};

// --
template <typename T>
class SurveyNode : public Node,
                   public Recv<PairNode<T>>,
                   public Send<T, PairNode<T>> {
  std::unique_ptr<Resolver<T>> _resolver;

public:
  SurveyNode(std::unique_ptr<Resolver<T>> &&r)
      : Node(OpenAsSurveyor), _resolver(std::forward<decltype(r)>(r)) {}

  Resolver<T> *resolver() const { return _resolver; }
};

// --

template <typename T>
class ResponseNode : public Node, public Reply<ResponseNode<T>> {
  std::unique_ptr<Replier<T>> _replier;

public:
  ResponseNode(std::unique_ptr<Replier<T>> &&r)
      : Node(OpenAsRespondent), _replier(std::forward<decltype(r)>(r)) {}

  Replier<T> *replier() const { return _replier.get(); }
};

// --

template <typename T>
class BusNode : public Node,
                public Recv<BusNode<T>>,
                public Send<T, BusNode<T>> {
  std::unique_ptr<Resolver<T>> _resolver;

public:
  BusNode(std::unique_ptr<Resolver<T>> &&r)
      : Node(OpenAsBus), _resolver(std::forward<decltype(r)>(r)) {}

  Resolver<T> *resolver() const { return _resolver; }
};

// --

} // namespace rpcpp
