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
    if (0 == r) {
      std::string result = ((D *)this)->resolver()->resolve(data, len, offset);
      nng_free(data, len);
    }
    return r;
  }
};

// --

template <typename D> class Reply {
public:
  int transact() {
    char *data = nullptr;
    size_t len = 0, offset = 0;
    std::string result;

    int r = ((D *)this)->recv(&data, &len);
    if (0 == r) {
      result = ((D *)this)->replier()->reply(data, len, offset);
      nng_free(data, len);
    }

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
  using TagType = T;

  PushNode() : Node(OpenAsPush) {}
};

// --

template <typename T>
class PublishNode : public Node, public Send<T, PublishNode<T>> {
public:
  using TagType = T;
  PublishNode() : Node(OpenAsPub) {}
};

// --

template <typename T> class PullNode : public Node, public Recv<PullNode<T>> {
  std::unique_ptr<Resolver<T>> _resolver;

public:
  using TagType = T;
  PullNode(std::unique_ptr<Resolver<T>> &&r)
      : Node(OpenAsPull), _resolver(std::forward<decltype(r)>(r)) {}

  Resolver<T> *resolver() const { return _resolver.get(); }
};

// --

template <typename T>
class SubscribeNode : public Node, public Recv<SubscribeNode<T>> {
  std::unique_ptr<Resolver<T>> _resolver;

public:
  using TagType = T;
  SubscribeNode(std::unique_ptr<Resolver<T>> &&r)
      : Node(OpenAsSub), _resolver(std::forward<decltype(r)>(r)) {
    setTopics("");
  }

  Resolver<T> *resolver() const { return _resolver.get(); }

  int setTopics(const std::string &topics) {
    return _sock.setOpt(NNG_OPT_SUB_SUBSCRIBE, topics.c_str(), topics.size());
  }
};

// --

template <typename T>
class RequestNode : public Node, public Request<T, RequestNode<T>> {
public:
  using TagType = T;
  RequestNode() : Node(OpenAsReq) {}
};

// --

template <typename T>
class ReplyNode : public Node, public Reply<ReplyNode<T>> {
  std::unique_ptr<Replier<T>> _replier;

public:
  using TagType = T;
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
  using TagType = T;
  PairNode(std::unique_ptr<Resolver<T>> &&r, nng_duration timeout_ms = 100)
      : Node(OpenAsPair), _resolver(std::forward<decltype(r)>(r)) {
    _sock.setOpt(NNG_OPT_RECVTIMEO, &timeout_ms, sizeof(timeout_ms));
  }

  Resolver<T> *resolver() const { return _resolver.get(); }
};

// --
template <typename T>
class SurveyNode : public Node,
                   public Recv<PairNode<T>>,
                   public Send<T, PairNode<T>> {
  std::unique_ptr<Resolver<T>> _resolver;

public:
  using TagType = T;
  SurveyNode(std::unique_ptr<Resolver<T>> &&r, nng_duration timeout_ms = 100)
      : Node(OpenAsSurveyor), _resolver(std::forward<decltype(r)>(r)) {
    _sock.setOpt(NNG_OPT_RECVTIMEO, &timeout_ms, sizeof(timeout_ms));
  }

  Resolver<T> *resolver() const { return _resolver.get(); }
};

// --

template <typename T>
class ResponseNode : public Node,
                     public Recv<ResponseNode<T>>,
                     public Send<T, ResponseNode<T>> {
  std::unique_ptr<Resolver<T>> _resolver;

public:
  using TagType = T;
  ResponseNode(std::unique_ptr<Resolver<T>> &&r)
      : Node(OpenAsRespondent), _resolver(std::forward<decltype(r)>(r)) {}

  Resolver<T> *resolver() const { return _resolver.get(); }
};

// --

template <typename T>
class BusNode : public Node,
                public Recv<BusNode<T>>,
                public Send<T, BusNode<T>> {
  std::unique_ptr<Resolver<T>> _resolver;

public:
  using TagType = T;
  BusNode(std::unique_ptr<Resolver<T>> &&r)
      : Node(OpenAsBus), _resolver(std::forward<decltype(r)>(r)) {}

  Resolver<T> *resolver() const { return _resolver.get(); }
};

// --

} // namespace rpcpp
