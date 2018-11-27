#include <iostream>
#include <string>
#include <sstream>
#include <chrono>
#include <thread>
#include <atomic>

#include <nanolog/nanolog.hpp>

#include <msgpack-c/msgpack.hpp>

#include <nng/nng.h>
#include <nng/protocol/reqrep0/rep.h>
#include <nng/protocol/pubsub0/pub.h>

#include "rpcpp.h"
#include "threadpool.h"
#include "node.h"
#include "nodeserver.h"
#include "task.h"

extern rpcpp::ThreadPool* g_poolServer;
extern rpcpp::Manager* g_omServer;

// --

// --

