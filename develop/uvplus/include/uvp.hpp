//
// uvp.hpp
//

#pragma once

#include <iostream>

#include "details/version.hpp"
#include "details/utilities.hpp"
#include "details/misc.hpp"
#include "details/eventloop.hpp"
#include "details/handle.hpp"
#include "details/thread.hpp"
#include "details/plugin.hpp"

namespace uvp {

inline void initialize(const char *path, const char *name,
                       UnsignedInt log_file_roll_size_mb,
                       UnsignedInt log_file_count = UINT32_MAX) {
  nlog::initialize(nlog::GuaranteedLogger(), path, name,
                      log_file_roll_size_mb, log_file_count);
  LOG_INFO << "libuv version: " << Version().str();
  std::cout << "libuv version: " << Version().str() << std::endl;
}
/*
class GuaranteedLogger {
  GuaranteedLogger(const char *path, const char *name,
         UnsignedInt log_file_roll_size_mb = 1,
         UnsignedInt log_file_count = 5) {
    nlog::initialize(nlog::GuaranteedLogger(), path, name,
                     log_file_roll_size_mb, log_file_count);
    LOG_INFO << "Guaranteed Logger starts. libuv version: " << Version().str();
  }
  ~GuaranteedLogger() {
    LOG_INFO << "Guaranteed Logger ends.";
  }
};
*/


} // namespace uvp
