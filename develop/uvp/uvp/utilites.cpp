#include <uv.h>
#include <misc.hpp>
#include <utilites.hpp>
#include <cxxabi.h>

/*
namespace std {
std::ostream &operator<<(std::ostream &stream, const rpcpp::FunType &ft) {
  stream << ft;
  return stream;
}
} // namespace std
*/
// --

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

// 处理libuv相关的错误
void log_error(int err, const char* fn, const char* fun, uint64_t ln) {
  Error e(err);
  nanolog::is_logged(nanolog::LogLevel::CRIT) &&
      nanolog::NanoLog() == nanolog::NanoLogLine(nanolog::LogLevel::CRIT, fn, fun, ln)
      << "err: " << err << ", name: " << e.name() << ", desc: " << e.strerror();
}

const char* demangle(const char *name)
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
  return s.c_str();
}
