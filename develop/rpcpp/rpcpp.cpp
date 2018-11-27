#include <iostream>
#include <string>

#include <cxxabi.h>

#include <nanolog/nanolog.hpp>

#include <nng/nng.h>

#include "resolver.h"
#include "rpcpp.h"

// --

namespace std {
std::ostream &operator<<(std::ostream &stream, const rpcpp::FunType &ft) {
  stream << ft;
  return stream;
}
} // namespace std

// --

namespace rpcpp {

// hex_dump is not a part of msgpack-c.
std::ostream &hex_dump(std::ostream &o, std::string const &v) {
  std::ios::fmtflags f(o.flags());
  o << std::hex;
  for (auto c : v) {
    o << "0x" << std::setw(2) << std::setfill('0')
      << (static_cast<int>(c) & 0xff) << ' ';
  }
  o.flags(f);
  return o;
}

// 处理nng相关的错误
void fatal(int err, const char* fn, const char* fun, uint64_t ln) {
  nanolog::is_logged(nanolog::LogLevel::CRIT) &&
      nanolog::NanoLog() == nanolog::NanoLogLine(nanolog::LogLevel::CRIT, fn, fun, ln)
      << "err: " << err << ", " << nng_strerror(err);
  if (err) {
    std::exit(1);
  }
}

std::string demangle(const char *name)
{
  char buf[1024] = {0};
  size_t len = sizeof(buf);
  int status = 0;
  abi::__cxa_demangle(name, buf, &len, &status);
  std::string s;
  if (status) {
    LOG_WARN << "abi::__cxa_demangle() return, status: " << status;
    s = name;
  } else {
    s = buf;
  }
  return s;
}

namespace server {


}
}
