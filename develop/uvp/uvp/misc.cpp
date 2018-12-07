#include <string>

#include <utilites.hpp>
#include <misc.hpp>

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

unsigned int Version::hex() const {
  return uv_version();
}

const char* Version::str() const {
  return uv_version_string();
}

// --

BufT allocBuf(size_t size) {
  BufT b;
  b.base = (char*)malloc(size);
  if (!b.base) {
    LOG_IF_ERROR_EXIT(UV_ENOMEM);
  }
  b.len = size;

  return b;
}

BufT copyToBuf(const char* p, size_t len) {
  BufT b = allocBuf(len);
  memcpy(b.base, p, len);
  return b;
}

BufT moveToBuf(char* p, size_t len) {
  BufT b;
  b.base = p;
  b.len = len;

  return b;
}

void freeBuf(BufT buf) {
  free(buf.base);
}

