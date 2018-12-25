//
// uvp.hpp
//

#pragma once

#include <details/version.hpp>
#include <details/utilities.hpp>
#include <details/misc.hpp>
#include <details/eventloop.hpp>
#include <details/handle.hpp>

namespace uvp {

inline void initialize(const char *path, const char *name,
                       UnsignedInt log_file_roll_size_mb,
                       UnsignedInt log_file_count = UINT32_MAX) {
  nanolog::initialize(nanolog::GuaranteedLogger(), path, name,
                      log_file_roll_size_mb, log_file_count);
  LOG_INFO << "libuv version: " << Version().str();
  std::cout << "libuv version: " << Version().str() << std::endl;
}

} // namespace uvp
