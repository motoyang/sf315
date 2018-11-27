//
// objects.h
//

#pragma once

#include <string>
#include <map>

#include "rpcpp.h"

namespace rpcpp {

class Object {
protected:
  std::string _name;

public:
  Object(const std::string &n) : _name(n) {}
  virtual ~Object() { LOG_INFO << "destory object: " << _name; }
  virtual const char *getName() { return _name.c_str(); }
  virtual void close() {}
};

// --

class Manager : public Object {
protected:
  std::map<std::string, std::unique_ptr<Object>> _map;

public:
  Manager(const std::string &n) : Object(n) {}

  bool add(std::unique_ptr<Object> &&o) {
    std::string n = o->getName();
    LOG_INFO << "add object: " << n;
    auto i = _map.insert(std::make_pair(n, std::forward<decltype(o)>(o)));
    LOG_FAIL_EXIT_1(i.second, "same object name not allowed: ", n);
    return i.second;
  }

  void close() {
    for (auto &i : _map) {
      i.second->close();
    }
  }

  Pointer getObjectPointer(const std::string &n) {
    auto i = _map.find(n);
    if (i == _map.end()) {
      LOG_WARN << "can't find object: " << n;
      return 0;
    }
    return (Pointer)i->second.get();
  }
};

} // namespace rpcpp
