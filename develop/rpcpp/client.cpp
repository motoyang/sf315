#include <string>
#include <iostream>
#include <sstream>

#include <nanolog/nanolog.hpp>

#include <msgpack.hpp>

#include <nng/nng.h>
#include <nng/protocol/reqrep0/req.h>

#include "anyarg.h"
#include "threadpool.h"
#include "nodeClient.h"
#include "client.h"

// --

namespace {

}
// --

std::threadpool* gp_client = nullptr;

namespace rpcpp2 {
  namespace client {


    void reqProcess(const char* url) {

      NodeRequest node(url);

      std::string fn1("repFun1");
      std::string fn2("repFun2");
      std::string fn3("query");
      std::string fn4("quit");
      int result;
      callOn(node, fn1, result, 55, std::string("client coming."));
      std::cout << "result: " << result << std::endl;
      callOn(node, fn2, result, 66, std::string("client2 coming2."));
      std::cout << "result2: " << result << std::endl;
      std::string info;
      callOn(node, fn3, info);
      std::cout << info << std::endl;
      callOn(node, fn4, result);
      std::cout << "result4: " << result << std::endl;
    }

    void subProcess(const char* url) {
      auto f = [](msgpack::object_handle const& oh) {
        std::string s = oh.get().as<std::string>();
        std::cout << "Received " << s.size() << " bytes:" << std::endl
                  << s << std::endl;
      };

      NodeSubscriber node(url);
      node.receive(f);
    }

    int startClient(const Anyarg &opt) {
      int r = 0;
      std::string url = opt.get_value_str('u');

      // 全局线程池，在本函数退出时，析构pool时，会退出所有线程
      std::threadpool pool(1);
      gp_client = &pool;

      gp_client->commit(subProcess, url.c_str());

//      reqProcess(url.c_str());
      return r;
    }
  }
}
