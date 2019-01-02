#pragma once

#include <string>

#include "types.hpp"
#include "utilities.hpp"

// --

namespace uvp {
/*
inline uv::BufT initBuf(char *base, unsigned int len) {
  return uv_buf_init(base, len);
}
*/

inline auto initBuf = uv_buf_init;

inline uv::BufT allocBuf(size_t size) {
  uv::BufT b = initBuf((char *)malloc(size), size);
  if (!b.base) {
    UVP_LOG_ERROR_EXIT(UV_ENOMEM);
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
  UVP_LOG_ERROR(r);

  char name[INET6_ADDRSTRLEN] = {0};
  r = uv_inet_ntop(AF_INET, (const void *)&paddr_in->sin_addr, name,
                   sizeof(name));
  UVP_LOG_ERROR(r);
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
  UVP_LOG_ERROR(r);

  char name[INET6_ADDRSTRLEN] = {0};
  r = uv_inet_ntop(AF_INET, (const void *)&paddr_in->sin_addr, name,
                   sizeof(name));
  UVP_LOG_ERROR(r);
  int port = ntohs(paddr_in->sin_port);

  std::stringstream ss;
  ss << name << ":" << port;
  return ss.str();
}
*/
// --

inline auto guessHandle = uv_guess_handle;
inline auto replaceAllocator = uv_replace_allocator;
inline auto bufInit = uv_buf_init;
inline auto setupArgs = uv_setup_args;
inline auto getProcessTitle = uv_get_process_title;
inline auto setProcessTitle = uv_set_process_title;
inline auto residentSetMemory = uv_resident_set_memory;
inline auto uptime = uv_uptime;
inline auto getrusage = uv_getrusage;
inline auto getpid = uv_os_getpid;
inline auto getppipd = uv_os_getppid;
inline auto cpuInfo = uv_cpu_info;
inline auto freeCpuInfo = uv_free_cpu_info;
inline auto interfaceAddresses = uv_interface_addresses;
inline auto freeInterfaceAddress = uv_free_interface_addresses;
inline auto loadavg = uv_loadavg;
inline auto ip4Addr = uv_ip4_addr;
inline auto ip6Addr = uv_ip6_addr;
inline auto ip4Name = uv_ip4_name;
inline auto ip6Name = uv_ip6_name;
inline auto inetNtop = uv_inet_ntop;
inline auto inetPton = uv_inet_pton;
inline auto ifIndextoname = uv_if_indextoname;
inline auto ifIndextoiid = uv_if_indextoiid;
inline auto exepath = uv_exepath;
inline auto cwd = uv_cwd;
inline auto chdir = uv_chdir;
inline auto osHomedir = uv_os_homedir;
inline auto osTmpdir = uv_os_tmpdir;
inline auto osGetPasswd = uv_os_get_passwd;
inline auto osFreePasswd = uv_os_free_passwd;
inline auto getTotleMemory = uv_get_total_memory;
inline auto hrtime = uv_hrtime;
inline auto osGetenv = uv_os_getenv;
inline auto osUnsetenv = uv_os_unsetenv;
inline auto osGethostname = uv_os_gethostname;
inline auto osGetpriority = uv_os_getpriority;
inline auto osSetpriority = uv_os_setpriority;

/*
inline int ip4Addr(const char *ip, int port, sockaddr_in *addr) {
  int r = uv_ip4_addr(ip, port, addr);
  UVP_LOG_ERROR(r);
  return r;
}

inline int ip6Addr(const char *ip, int port, sockaddr_in6 *addr) {
  int r = uv_ip6_addr(ip, port, addr);
  UVP_LOG_ERROR(r);
  return r;
}

inline int ip4Name(const struct sockaddr_in* src, char* dst, size_t size) {
  int r = uv_ip4_name(src, dst, size);
  UVP_LOG_ERROR(r);
  return r;
}

inline int ip6Name(const struct sockaddr_in6* src, char* dst, size_t size) {
  int r= uv_ip6_name(src, dst, size);
  UVP_LOG_ERROR(r);
  return r;
}

inline int interfaceAddresses(uv::InterfaceAddress **addresses, int *count) {
  int r = uv_interface_addresses(addresses, count);
  UVP_LOG_ERROR(r);
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
  UVP_LOG_ERROR(r);
  return r;
}

inline int cpuInfo(uvp::uv::CpuInfoT** cpu_infos, int *count) {
  int r= uv_cpu_info(cpu_infos, count);
  UVP_LOG_ERROR(r);
  return r;
}

inline void freeCpuInfo(uvp::uv::CpuInfoT* cpu_infos, int count) {
  uv_free_cpu_info(cpu_infos, count);
}

inline uvp::uv::HandleType guessHandle(uvp::uv::File file) {
  return uv_guess_handle(file);
}
*/


} // namespace uvp
