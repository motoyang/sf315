#include <string>
#include <iostream>

#include <utilites.hpp>
#include <misc.hpp>
#include <uv.hpp>

// --

int Error::translateSysError(int sys_errno) {
  return uv_translate_sys_error(sys_errno);
}

Error::Error(int err) : _err(err) {}

Error::~Error() {}

std::string Error::strerror() const {
  // return uv_strerror(_err);
  const int len = 1024;
  std::string s(len, 0);
  uv_strerror_r(_err, s.data(), s.size());
  return s;
}

std::string Error::name() const {
  // return uv_err_name(_err);
  const int len = 512;
  std::string s(len, 0);
  uv_err_name_r(_err, s.data(), s.size());
  return s;
}

// --

unsigned int Version::hex() const { return uv_version(); }

const char *Version::str() const { return uv_version_string(); }

// --
static std::atomic<size_t> nbuf_alloc = 0;

BufT allocBuf(size_t size) {
  BufT b;
  b.base = (char *)malloc(size);
  if (!b.base) {
    LOG_IF_ERROR_EXIT(UV_ENOMEM);
  }
  b.len = size;

  ++nbuf_alloc;

  // if (nbuf_alloc > 10)
  //  std::cout << "nbuf_alloc: " << nbuf_alloc << std::endl;
  
  return b;
}

BufT copyToBuf(const char *p, size_t len) {
  BufT b = allocBuf(len);
  memcpy(b.base, p, len);
  return b;
}

BufT moveToBuf(char *p, size_t len) {
  BufT b;
  b.base = p;
  b.len = len;

  return b;
}

void freeBuf(BufT buf) {
  if (buf.base) {
    free(buf.base);
    --nbuf_alloc;
  //  std::cout << "nbuf_free: " << nbuf_alloc << std::endl;

  }
}

void bufCount() {
  std::cout << "buf alloced: " << nbuf_alloc << std::endl;
}

// --

// 1st paramater: AF_INET or AF_INET6 for ipv6
std::string nameOfPeer(int af, TcpI *tcp) {
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
std::string nameOfSock(int af, TcpI *tcp) {
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
