#include <iostream>
#include <memory>

#include <uvp.hpp>

#define DEFAULT_PORT 7000

int main(int argc, char *argv[]) {
  uvp::initialize("./", "interfaces", 1, 3);

  char buf[512] = {0};
  uvp::uv::InterfaceAddress *info = nullptr;
  int count = 0;
  uvp::interfaceAddresses(&info, &count);
  int i = count;

  while (i--) {
    uvp::uv::InterfaceAddress interface = info[i];

    std::cout << "Name: " << interface.name << std::endl;
    std::cout << "Internal? " << (interface.is_internal? "Yes": "No") << std::endl;
    if (interface.address.address4.sin_family == AF_INET) {
      uvp::ip4Name(&interface.address.address4, buf, sizeof(buf));
      std::cout << "IPV4 address: " << buf << std::endl;
    } else if (interface.address.address6.sin6_family == AF_INET6) {
      uvp::ip6Name(&interface.address.address6, buf, sizeof(buf));
      std::cout << "IPV6 address: " << buf << std::endl;
    }

    std::cout << std::endl;
  }

  uvp::freeInterfaceAddress(info, count);

  LOG_INFO << "quit with return code: " << 0;
  return 0;
}
