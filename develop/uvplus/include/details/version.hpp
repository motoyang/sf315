#pragma once

#include "types.hpp"

namespace uvp {

class Version {
public:
  unsigned int hex() const { return uv_version(); }
  const char *str() const { return uv_version_string(); }
};

} // namespace uvp
