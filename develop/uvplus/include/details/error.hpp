#pragma once

#include <details/types.hpp>

#include <string>

namespace uvp {

class Error {
  int _err;

public:
  Error(int err) : _err(err) {}
  virtual ~Error() {}

  static int translateSysError(int sys_errno) {
    return uv_translate_sys_error(sys_errno);
  }

  std::string strerror() const {
    const int len = 1024;
    std::string s(len, 0);
    uv_strerror_r(_err, s.data(), s.size());
    return s;
  }

  std::string name() const {
    const int len = 512;
    std::string s(len, 0);
    uv_err_name_r(_err, s.data(), s.size());
    return s;
  }
};

} // namespace uvp
