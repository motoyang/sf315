#pragma once

#include <functional>

#include <details/types.hpp>
#include <details/misc.hpp>

namespace uvp {

class Lib {
  mutable uvp::uv::LibT _lib;

public:
  virtual ~Lib() {}

  int open(const char *filename) {
    int r = uv_dlopen(filename, &_lib);
    if (r == -1) {
      LOG_CRIT << "dlerror: " << dlerror();
    }
    return r;
  }

  void close() { uv_dlclose(&_lib); }

  int dlsym(const char *name, void **ptr) {
    int r = uv_dlsym(&_lib, name, ptr);
    if (r == -1) {
      LOG_CRIT << "dlerror: " << dlerror();
    }
    return r;
  }

  const char *dlerror() { return uv_dlerror(&_lib); }
};

} // namespace uvp
