//
// node.h
//

#pragma once

// --

namespace rpcpp2 {
  namespace server {

    void pubProcess(const char *url,
                    std::function<std::string()> &&f);
    void repProcess(const char *url,
                    std::function<std::string(const char*, size_t len)>&& f);


  }
}
