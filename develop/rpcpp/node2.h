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
#include "objects.h"

// --

namespace rpcpp {

// --

template <typename T, typename D> class Send2 {
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

template <typename D> class Recv2 {
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

public:
  int watch () {
    while (((D*)this)->isRunning()) {
      transact();
    }
    return 0;
  }
};

// --

template <typename D> class Reply2 {
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
public:
  int watch() {
    while (((D*)this)->isRunning()) {
      transact();
    }
    return 0;
  }
};

template <typename T, typename D> class Request2 {
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

class Node2 : public Object {
protected:
  Socket _sock;
  int recv(char **buf, size_t *len);
  int send(void *data, size_t len);

public:
  Node2(const std::string &n, Socket::OpenFun f);
  virtual ~Node2() {}

  bool isRunning() const;
  int listen(const char *url);
  int dial(const char *url);
  const char *strerror(int e);
  virtual void close();
};

// --

template <typename T>
class PushNode2 : public Node2, public Send2<T, PushNode2<T>> {
  friend class Send2<T, PushNode2<T>>;

public:
  PushNode2(const std::string &n) : Node2(n, OpenAsPush) {}
};

// --

template <typename T>
class PublishNode2 : public Node2, public Send2<T, PublishNode2<T>> {
  friend class Send2<T, PublishNode2<T>>;

public:
  PublishNode2(const std::string &n) : Node2(n, OpenAsPub) {}
};

// --

template <typename T>
class PullNode2 : public Node2, public Recv2<PullNode2<T>> {
  friend class Recv2<PullNode2<T>>;
  std::unique_ptr<Resolver<T>> _resolver;

public:
  PullNode2(const std::string &n, std::unique_ptr<Resolver<T>> &&r)
      : Node2(n, OpenAsPull), _resolver(std::forward<decltype(r)>(r)) {}

  Resolver<T> *resolver() const { return _resolver.get(); }
};

// --

template <typename T>
class SubscribeNode2 : public Node2, public Recv2<SubscribeNode2<T>> {
  friend class Recv2<SubscribeNode2<T>>;
  std::unique_ptr<Resolver<T>> _resolver;

public:
  SubscribeNode2(const std::string &n, std::unique_ptr<Resolver<T>> &&r,
                 const std::string &topics = "")
      : Node2(n, OpenAsSub), _resolver(std::forward<decltype(r)>(r)) {
    _sock.setOpt(NNG_OPT_SUB_SUBSCRIBE, topics.c_str(), topics.size());
  }

  Resolver<T> *resolver() const { return _resolver.get(); }
};

// --

template <typename T>
class RequestNode2 : public Node2, public Request2<T, RequestNode2<T>> {
  friend class Request2<T, RequestNode2<T>>;

public:
  RequestNode2(const std::string &n) : Node2(n, OpenAsReq) {}
};

// --

template <typename T>
class ReplyNode2 : public Node2, public Reply2<ReplyNode2<T>> {
  friend class Reply2<ReplyNode2<T>>;
  std::unique_ptr<Replier<T>> _replier;

public:
  ReplyNode2(const std::string &n, std::unique_ptr<Replier<T>> &&r)
      : Node2(n, OpenAsRep), _replier(std::forward<decltype(r)>(r)) {}

  Replier<T> *replier() const { return _replier.get(); }
};

// --

template <typename T>
class PairNode2 : public Node2,
                  public Recv2<PairNode2<T>>,
                  public Send2<T, PairNode2<T>> {
  friend class Recv2<PairNode2<T>>;
  friend class Send2<T, PairNode2<T>>;
  std::unique_ptr<Resolver<T>> _resolver;

public:
  using TagType = T;
  PairNode2(const std::string &n, std::unique_ptr<Resolver<T>> &&r,
            nng_duration timeout_ms = 100)
      : Node2(n, OpenAsPair), _resolver(std::forward<decltype(r)>(r)) {
    _sock.setOpt(NNG_OPT_RECVTIMEO, &timeout_ms, sizeof(timeout_ms));
  }

  Resolver<T> *resolver() const { return _resolver.get(); }
};

// --
template <typename T>
class SurveyNode2 : public Node2,
                    public Recv2<SurveyNode2<T>>,
                    public Send2<T, SurveyNode2<T>> {
  friend class Recv2<SurveyNode2<T>>;
  friend class Send2<T, SurveyNode2<T>>;
  std::unique_ptr<Resolver<T>> _resolver;

public:
  SurveyNode2(const std::string &n, std::unique_ptr<Resolver<T>> &&r,
              nng_duration timeout_ms = 1000)
      : Node2(n, OpenAsSurveyor), _resolver(std::forward<decltype(r)>(r)) {
    _sock.setOpt(NNG_OPT_RECVTIMEO, &timeout_ms, sizeof(timeout_ms));
  }

  Resolver<T> *resolver() const { return _resolver.get(); }
};

// --

template <typename T>
class ResponseNode2 : public Node2,
                      public Recv2<ResponseNode2<T>>,
                      public Send2<T, ResponseNode2<T>> {
  friend class Recv<ResponseNode2<T>>;
  friend class Send<T, ResponseNode2<T>>;
  std::unique_ptr<Resolver<T>> _resolver;

public:
  using TagType = T;
  ResponseNode2(const std::string &n, std::unique_ptr<Resolver<T>> &&r,
                nng_duration timeout_ms = 100)
      : Node2(n, OpenAsRespondent), _resolver(std::forward<decltype(r)>(r)) {
    _sock.setOpt(NNG_OPT_RECVTIMEO, &timeout_ms, sizeof(timeout_ms));
  }

  Resolver<T> *resolver() const { return _resolver.get(); }
};

// --

template <typename T>
class BusNode2 : public Node2,
                 public Recv2<BusNode2<T>>,
                 public Send2<T, BusNode2<T>> {
  friend class Recv2<BusNode2<T>>;
  friend class Send2<T, BusNode2<T>>;
  std::unique_ptr<Resolver<T>> _resolver;

public:
  BusNode2(const std::string &n, std::unique_ptr<Resolver<T>> &&r,
          nng_duration timeout_ms = 100)
      : Node2(n, OpenAsBus), _resolver(std::forward<decltype(r)>(r)) {
    _sock.setOpt(NNG_OPT_RECVTIMEO, &timeout_ms, sizeof(timeout_ms));
  }

  Resolver<T> *resolver() const { return _resolver.get(); }
};

// --

} // namespace rpcpp
