#include <iostream>
#include <string>

#include <nanolog/nanolog.hpp>

#include <nng/nng.h>

#include "rpcpp.h"

// --

namespace rpcpp2 {

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
void fatal(int err, const char* fn, const char* fun, uint ln) {
  nanolog::is_logged(nanolog::LogLevel::CRIT) &&
      nanolog::NanoLog() == nanolog::NanoLogLine(nanolog::LogLevel::CRIT, fn, fun, ln)
      << "err: " << err << ", " << nng_strerror(err);
  if (err) {
    exit(1);
  }
}

namespace server {


}
}
