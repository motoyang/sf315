#include <uv.h>
#include <types.hpp>

#include <string>

// --

class Error {
  int _err;

public:
  Error(int err);
  virtual ~Error();

  static int translateSysError(int sys_errno);

  std::string strerror() const;
  std::string name() const;
};

// --

class Version {
public:
  unsigned int hex() const;
  const char* str() const;

};

// --

BufT allocBuf(size_t size);
BufT copyToBuf(const char* p, size_t len);
BufT moveToBuf(const char* p, size_t len);
void freeBuf(BufT buf);