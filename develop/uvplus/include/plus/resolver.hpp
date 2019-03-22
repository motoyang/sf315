#pragma once

#include <iostream>
#include <string>
#include <map>
#include <unordered_map>

#include <msgpack.hpp>

#include <uvp.hpp>

#include "functraits.h"

// --

namespace uvplus {

// --

using FunType = std::string (*)(uvp::Pointer, const char *, std::size_t,
                                std::size_t, std::size_t);
using ValueType = std::pair<FunType, uvp::Pointer>;

// --

template <typename T>
class Resolver {
 protected:
  std::unordered_map<T, ValueType> _map;
  std::map<T, std::string> _queryMap;

 public:
  template <typename FP>
  Resolver<T> &defineFun(T const &tag, FP const pf) {
    ValueType v = std::make_pair(&Call3<FP>::f, *(uvp::Pointer *)(&pf));

    if (auto i = _map.insert(std::make_pair(tag, v)); !i.second) {
      LOG_CRIT << "same tag is not allowed, tag: " << tag;
    } else {
      _queryMap.insert(std::make_pair(tag, uvp::demangle(typeid(pf).name())));
    }

    return *this;
  }

  std::string resolve(const char *buf, size_t len, size_t &off) {
    T tag;
    msgpack::object_handle oh = msgpack::unpack(buf, len, off);
    oh.get().convert(tag);

    std::string result;
    if (auto i = _map.find(tag); i == _map.end()) {
      LOG_WARN << "can't find tag: " << tag;
    } else {
      auto f = i->second.first;
      result = f(i->second.second, buf, len, off, 0);
    }

    return result;
  }

  void show(std::ostream& os) const {
    os << "functions defined: " << std::endl << _queryMap << std::endl;
  }
};

// --

template <typename T>
class Replier;
template <typename T>
class Class {
  friend class Replier<T>;

  T _tag;
  std::unordered_map<T, ValueType> &_map;
  std::map<T, std::string> &_queryMap;

 protected:
  explicit Class(T const &tag, std::unordered_map<std::string, ValueType> &cm,
                 std::map<std::string, std::string> &qm)
      : _tag(tag), _map(cm), _queryMap(qm) {}

 public:
  template <typename FP>
  Class &defineMethod(T const &tag, FP const &pf) {
    if (auto i = _queryMap.insert(
            std::make_pair(tag, uvp::demangle(typeid(pf).name())));
        !i.second) {
      LOG_CRIT << "same tag is not allowed, tag: " << tag;
    } else {
      auto p = std::make_unique<FP>(pf);
      ValueType v =
          std::make_pair(&CallMember3<FP>::f, (uvp::Pointer)(p.release()));
      _map.insert(std::make_pair(tag, v));
    }

    return *this;
  }
};

// --

template <typename T>
class Replier : public Resolver<T> {
  std::unordered_map<T, std::unordered_map<T, ValueType>> _mapClasses;
  std::map<T, std::map<T, std::string>> _queryClassesMap;

 public:
  virtual ~Replier() {
    // 删除成员函数的存储空间，这些空间是在Class::defineMethod()中申请的。
    for (auto &c : _mapClasses) {
      for (auto &f : c.second) {
        char *p = (char *)f.second.second;
        delete[] p;
      }
    }
  }

  Class<T> defineClass(T const &tag) {
    if (_mapClasses.find(tag) != _mapClasses.end()) {
      LOG_CRIT << "same tag is not allowed, tag: " << tag;
    } else {
      _mapClasses.emplace(tag, std::unordered_map<T, ValueType>());
      _queryClassesMap.emplace(tag, std::map<T, std::string>());
    }

    return Class<T>(tag, _mapClasses.at(tag), _queryClassesMap.at(tag));
  }

  std::string reply(const char *buf, size_t len, size_t &off) {
    size_t off_origin = off;
    msgpack::object_handle oh = msgpack::unpack(buf, len, off);
    T tag;
    oh.get().convert(tag);

    // 先按照tag1查找，如果找不到，就作为唯一tag在resolver中处理。
    auto i = _mapClasses.find(tag);
    if (i == _mapClasses.end()) {
      off = off_origin;
      return Resolver<T>::resolve(buf, len, off);
    }
    auto &m = i->second;

    // 取第二个tag
    msgpack::object_handle oh2 = msgpack::unpack(buf, len, off);
    T tag2;
    oh2.get().convert(tag2);

    // 查找第二个tag，并调用对应的函数
    auto i2 = m.find(tag2);
    if (i2 == m.end()) {
      LOG_WARN << "can't find tag2: " << tag2 << "in tag1: " << tag;
      return std::string();
    }
    auto f = i2->second.first;
    std::string result = f(i2->second.second, buf, len, off, 0);

    return result;
  }

  void show(std::ostream& os) const {
    Resolver<T>::show(os);
    os << "classes defined: " << std::endl << _queryClassesMap << std::endl;
  }
};

}  // namespace uvplus
