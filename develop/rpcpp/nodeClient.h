//
// nodeClient.h
//

#pragma once

// --

namespace rpcpp2 {
  namespace client {

    class NodeSubscriber {
      nng_socket _sock;

    public:
      NodeSubscriber(const char* url);
      ~NodeSubscriber();

      void close();
      void receive(std::function<void(msgpack::object_handle const&)>&& f) const;
    };

    class NodeRequest {
      nng_socket _sock;

    public:
      NodeRequest(const char* url);
      ~NodeRequest();
      msgpack::object_handle request(const std::string& s) const;
    };

    template <typename R, typename... Args>
    int callOn(const NodeRequest& node, const std::string &fn, R &result, Args... args) {
      std::stringstream ss;
      msgpack::pack(ss, fn);
      ((msgpack::pack(ss, args), ...));

      msgpack::object_handle oh = node.request(ss.str());
      oh.get().convert(result);

      return 0;
    }
  }
}
