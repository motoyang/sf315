#pragma once

#include <string>

inline constexpr const char* ConfigFilename = "config.json";

struct JsonConfig {
  std::string server;
  unsigned short server_port;
  std::string local_address;
  unsigned short local_port;
  std::string password;
  int timeout;
  std::string method;
  bool fast_open;
};

bool readConfig(const std::string& fn, JsonConfig& jc);