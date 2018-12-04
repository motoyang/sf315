#include <uv.h>

#include <types.hpp>

// --

class Error {
  int _err;

public:
  Error(int err);
  virtual ~Error();

  static int translateSysError(int sys_errno);

  const char* strerror() const;
  const char* name() const;
};

// --

class Version {
public:
  unsigned int hex() const;
  const char* str() const;

};