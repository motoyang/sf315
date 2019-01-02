#include <iostream>
#include <memory>

#include <uvp.hpp>

void onRead(uvp::Stream *client, ssize_t nread, const uv_buf_t *buf) {
  if (nread < 0) {
    if (nread != UV_EOF) {
      std::cerr << "read error: " << uvp::Error(nread).strerror() << std::endl;
    }
    client->close(nullptr);
  } else if (nread > 0) {
    std::string data(buf->base, nread);
    std::cout << data;
  }
}

void onConnect(uvp::Stream *client, int status) {
  if (status < 0) {
    std::cout << "connect failed error: " << uvp::Error(status).strerror()
              << std::endl;
    return;
  }

  client->readStart(nullptr, onRead);
}

int main(int argc, char *argv[]) {
  uvp::initialize("./", "dns", 1, 3);
  auto l = std::make_unique<uvp::LoopObject>();

  auto tcpSocket = std::make_unique<uvp::TcpObject>(l.get());
  auto onResolver = [&tcpSocket](uvp::Getaddrinfo *req, int status,
                                 addrinfo *res) {
    if (status < 0) {
      std::cerr << "getaddrinfo callback error: "
                << uvp::Error(status).strerror() << std::endl;
      return;
    }

    char addr[17] = {0};
    uvp::ip4Name((const sockaddr_in *)res->ai_addr, addr, sizeof(addr) - 1);
    std::cout << addr << std::endl;

    tcpSocket->connect((const sockaddr *)res->ai_addr, onConnect);
  };

  addrinfo hints = {0};
  hints.ai_family = PF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = 0;

  uvp::GetaddrinfoReq resolver;
  std::cout << "irc.freenode.net is... ";
  int r =
      l->getAddrInfo(&resolver, onResolver, "irc.freenode.net", "6667", &hints);

  if (r) {
    std::cerr << "getaddrinfo call error: " << uvp::Error(r).strerror()
              << std::endl;
  }

  r = l->run(UV_RUN_DEFAULT);
  r = l->close();

  LOG_INFO << "quit with return code: " << r;
  return r;
}
