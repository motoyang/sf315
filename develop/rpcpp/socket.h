#pragma once

#include "nng/nng.h"
#include "nng/protocol/bus0/bus.h"
#include "nng/protocol/pair0/pair.h"
#include "nng/protocol/pipeline0/pull.h"
#include "nng/protocol/pipeline0/push.h"
#include "nng/protocol/pubsub0/pub.h"
#include "nng/protocol/pubsub0/sub.h"
#include "nng/protocol/reqrep0/rep.h"
#include "nng/protocol/reqrep0/req.h"
#include "nng/protocol/survey0/respond.h"
#include "nng/protocol/survey0/survey.h"

// --

class Socket {
  nng_socket _sock;

public:
  using OpenFun = int (*)(nng_socket *);

  Socket(OpenFun f);

  int close();
  int dial(const char *url);
  int listen(const char *url);
  int recv(void *data, size_t *len);
  int send(void *data, size_t len);
  int setOpt(const char *opt, const void *val, size_t valsz);
  int getOpt(const char *opt, void *val, size_t *valszp);
  int socketId() const;
};

// --

extern Socket::OpenFun OpenAsReq;
extern Socket::OpenFun OpenAsRep;
extern Socket::OpenFun OpenAsPub;
extern Socket::OpenFun OpenAsSub;
extern Socket::OpenFun OpenAsPull;
extern Socket::OpenFun OpenAsPush;
extern Socket::OpenFun OpenAsPair;
extern Socket::OpenFun OpenAsBus;
extern Socket::OpenFun OpenAsSurveyor;
extern Socket::OpenFun OpenAsRespondent;
