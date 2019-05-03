#include <iomanip>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include "readjson.h"

using json = nlohmann::json;

// --

static constexpr const char* J_server = "server";
static constexpr const char* J_server_port = "server_port";
static constexpr const char* J_local_address = "local_address";
static constexpr const char* J_local_port = "local_port";
static constexpr const char* J_password = "password";
static constexpr const char* J_timeout = "timeout";
static constexpr const char* J_method = "method";
static constexpr const char* J_fast_open = "fast_open";

void to_json(json &j, const JsonConfig &jc) {
  j = json{
    {J_server, jc.server},
    {J_server_port, jc.server_port},
    {J_local_address, jc.local_address},
    {J_local_port, jc.local_port},
    {J_password, jc.password},
    {J_timeout, jc.timeout},
    {J_method, jc.method},
    {J_fast_open, jc.fast_open}
    };
}

void from_json(const json &j, JsonConfig& jc) {
  j.at("server").get_to(jc.server);
  j.at(J_server_port).get_to(jc.server_port);
  j.at(J_local_address).get_to(jc.local_address);
  j.at(J_local_port).get_to(jc.local_port);
  j.at(J_password).get_to(jc.password);
  j.at(J_timeout).get_to(jc.timeout);
  j.at(J_method).get_to(jc.method);
  j.at(J_fast_open).get_to(jc.fast_open);
}

// --

bool readConfig(const std::string &fn, JsonConfig &jc) {
  // read a JSON file
  std::ifstream i(fn);
  json j;
  i >> j;

  std::cout << std::setw(2) << j << std::endl;

  j.get_to(jc);
  return true;
}