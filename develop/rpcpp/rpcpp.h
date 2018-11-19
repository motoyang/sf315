//
// rpcpp.h
//

#pragma once

// --

#define FATAL_EXIT(err) fatal(err, __FILE__, __func__, __LINE__)
#define LOG_TRACK (LOG_INFO << " haha...")

// --

namespace rpcpp2 {

void fatal(int err, const char* fn, const char* fun, uint ln);
std::ostream &hex_dump(std::ostream &o, std::string const &v);

}
