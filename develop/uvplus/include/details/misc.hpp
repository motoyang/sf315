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
  UVP_ASSERT(buf.base);
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

inline int ip4Addr(const char *ip, int port, sockaddr_in *addr) {
  int r = uv_ip4_addr(ip, port, addr);
  LOG_IF_ERROR(r);
  return r;
}

inline int ip6Addr(const char *ip, int port, sockaddr_in6 *addr) {
  int r = uv_ip6_addr(ip, port, addr);
  LOG_IF_ERROR(r);
  return r;
}

inline int ip4Name(const struct sockaddr_in* src, char* dst, size_t size) {
  int r = uv_ip4_name(src, dst, size);
  LOG_IF_ERROR(r);
  return r;
}

inline int ip6Name(const struct sockaddr_in6* src, char* dst, size_t size) {
  int r= uv_ip6_name(src, dst, size);
  LOG_IF_ERROR(r);
  return r;
}

inline int interfaceAddresses(uv::InterfaceAddress **addresses, int *count) {
  int r = uv_interface_addresses(addresses, count);
  LOG_IF_ERROR(r);
  return r;
}

inline void freeInterfaceAddress(uv::InterfaceAddress* addresses, int count) {
  return uv_free_interface_addresses(addresses, count);
}

inline uv::PidT getpid() {
  return uv_os_getpid();
}

inline int exepath(char* buffer, size_t* size) {
  int r = uv_exepath(buffer, size);
  LOG_IF_ERROR(r);
  return r;
}

inline int cpuInfo(uvp::uv::CpuInfoT** cpu_infos, int *count) {
  int r= uv_cpu_info(cpu_infos, count);
  LOG_IF_ERROR(r);
  return r;
}

inline void freeCpuInfo(uvp::uv::CpuInfoT* cpu_infos, int count) {
  uv_free_cpu_info(cpu_infos, count);
}

} // namespace uvp
