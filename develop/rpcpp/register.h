#pragma once

#include <functional>
#include <optional>
#include <ostream>
#include <string>
#include <map>
#include <unordered_map>
#include <utility>
#include <vector>

#include <cxxabi.h>

#include "functraits.h"

// --

namespace rpcpp2 {
using PointerType = std::ptrdiff_t;
using FunType = std::function<std::string(PointerType, const char *,
                                          std::size_t, std::size_t)>;
using ValueType = std::pair<FunType, PointerType>;

namespace server {

  std::string demangle(const char* name);


class Register;
class Class {
  friend class Register;

  std::string _name;
  std::unordered_map<std::string, ValueType> &_map;
  std::map<std::string, std::string> &_queryMap;

protected:
  explicit Class(const std::string &cn,
                 std::unordered_map<std::string, ValueType> &&nm,
                 std::map<std::string, std::string> &&qm)
      : _name(cn), _map(nm), _queryMap(qm) {}

public:
  template <typename FP> Class& exportMethod(const std::string &mn, FP const pf) {
    std::string name = _name + "::" + mn;
    _queryMap.insert(std::make_pair(name, demangle(typeid(pf).name())));

    ValueType v = std::make_pair(&CallMember3<FP>::f, *(PointerType *)(&pf));
    _map.insert(std::make_pair(name, v));
    return *this;
  }
};

class Register {
  std::string _name;
  std::unordered_map<std::string, ValueType> _map;
  std::map<std::string, std::string> _queryFunMap;
  std::map<std::string, std::string> _queryMethodMap;

public:
  static Register &instance() {
    static Register r(std::string("R1"));
    return r;
  }

  explicit Register(const std::string &name) : _name(name) {}

  Class defineClass(const std::string &cn) {
    return Class(cn, std::forward<decltype(_map)>(_map),
                 std::forward<decltype(_queryMethodMap)>(_queryMethodMap));
  }

  template <typename FP> Register& exportFun(const std::string &name, FP const pf) {
    _queryFunMap.insert(std::make_pair(name, demangle(typeid(pf).name())));

    ValueType v =
        std::make_pair(&Call3<FP>::f, reinterpret_cast<PointerType>(pf));
    _map.insert(std::make_pair(name, v));
    return *this;
  }

  std::optional<ValueType> find(const std::string &name) {
    std::optional<ValueType> r;
    auto i = _map.find(name);
    if (i != _map.end()) {
      r = i->second;
    }
    return r;
  }

  void show() const;
  std::string query() const;
};

} // namespace server
} // namespace rpcpp2
