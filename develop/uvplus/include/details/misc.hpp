#pragma once

#include <details/types.hpp>
#include <details/utilities.hpp>

#include <string>

// --

namespace uvp {

inline uv::BufT initBuf(char *base, unsigned int len) {
  return uv_buf_init(base, len);
}

inline uv::BufT allocBuf(size_t size) {
  uv::BufT b = initBuf((char *)malloc(size), size);
  if (!b.base) {
    LOG_IF_ERROR_EXIT(UV_ENOMEM);
  }
  return b;
}

inline uv::BufT copyToBuf(const char *p, size_t len) {
  uv::BufT b = allocBuf(len);
  memcpy(b.base, p, len);
  return b;
}

inline uv::BufT moveToBuf(char *p, size_t len) {
  uv::BufT b;
  b.base = p;
  b.len = len;

  return b;
}

inline void freeBuf(uv::BufT buf) {
  UVP_ASSERT(!buf.base);
  free(buf.base);
}

// --
/*
class TcpI;

// 1st paramater: AF_INET or AF_INET6 for ipv6
inline std::string nameOfPeer(int af, TcpI *tcp) {
  sockaddr addr;
  sockaddr_in *paddr_in = (sockaddr_in *)&addr;

  int len = sizeof(addr);
  int r = tcp->getpeername(&addr, &len);
  LOG_IF_ERROR(r);

  char name[INET6_ADDRSTRLEN] = {0};
  r = uv_inet_ntop(AF_INET, (const void *)&paddr_in->sin_addr, name,
                   sizeof(name));
  LOG_IF_ERROR(r);
  int port = ntohs(paddr_in->sin_port);

  std::stringstream ss;
  ss << name << ":" << port;
  return ss.str();
}

// 1st paramater: AF_INET or AF_INET6 for ipv6
inline std::string nameOfSock(int af, TcpI *tcp) {
  sockaddr addr;
  sockaddr_in *paddr_in = (sockaddr_in *)&addr;

  int len = sizeof(addr);
  int r = tcp->getsockname(&addr, &len);
  LOG_IF_ERROR(r);

  char name[INET6_ADDRSTRLEN] = {0};
  r = uv_inet_ntop(AF_INET, (const void *)&paddr_in->sin_addr, name,
                   sizeof(name));
  LOG_IF_ERROR(r);
  int port = ntohs(paddr_in->sin_port);

  std::stringstream ss;
  ss << name << ":" << port;
  return ss.str();
}
*/
// --

} // namespace uvp
