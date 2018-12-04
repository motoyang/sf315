#include <misc.hpp>

#include <string>

// --

int Error::translateSysError(int sys_errno) {
  return uv_translate_sys_error(sys_errno);
}

Error::Error(int err) : _err(err) {}

Error::~Error() {}

const char *Error::strerror() const {
  const int len = 1024;
  std::string s(len, 0);
  uv_strerror_r(_err, s.data(), s.size());
  return s.c_str();
}

const char* Error::name() const {
  const int len = 512;
  std::string s(len, 0);
  uv_err_name_r(_err, s.data(), s.size());
  return s.c_str();
}

// --

unsigned int Version::hex() const {
  return uv_version();
}

const char* Version::str() const {
  return uv_version_string();
}

// --