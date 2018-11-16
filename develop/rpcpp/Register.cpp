#include <functional>
#include <iostream>
#include <sstream>
#include <unordered_map>

#include <msgpack.hpp>
#include <nanolog/nanolog.hpp>
#include <nng/nng.h>
#include <nng/protocol/reqrep0/rep.h>
#include <nng/protocol/reqrep0/req.h>

#include <pp/prettyprint.h>
#include "Register.h"

namespace std {

std::ostream &operator<<(std::ostream &stream, const rpcpp2::FunType &ft) {
  stream << &ft;
  return stream;
}

} // namespace std

namespace rpcpp2 {
namespace server {

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

void Register::show() const {
  std::cout << _map << std::endl;
  std::cout << _queryFunMap << std::endl;
  std::cout << _queryMethodMap << std::endl;
}

std::string Register::query() const
{
  std::stringstream ss;
  ss << "Functions: " << std::endl;
  for (auto const& i: _queryFunMap) {
    ss << i.first << ": " << i.second << std::endl;
  }

  ss << "Methods: " << std::endl;
  for (auto const& i: _queryMethodMap) {
    ss << i.first << ": " << i.second << std::endl;
  }
\
  return ss.str();
}

} // namespace server
} // namespace rpcpp2
