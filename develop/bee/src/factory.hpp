#pragma once

// --

#include <memory>
#include <unordered_map>
#include <functional>
#include <cassert>

// --

template <typename Product, typename Id, typename... Args> class Factory {
public:
  using CreateMethod = std::function<std::unique_ptr<Product>(Args &&...)>;

  Factory() = delete;

  static bool registerId(Id id, CreateMethod creator) {
    auto &map = mapOfId();
    auto it = map.insert({id, creator});
    return it.second;
  }

  static std::unique_ptr<Product> create(Id id, Args &&... args) {
    auto &map = mapOfId();
    if (auto it = map.find(id); it != map.end())
      return it->second(std::forward<Args>(args)...);

    return nullptr;
  }

private:
  static std::unordered_map<Id, CreateMethod> &mapOfId() {
    static std::unordered_map<Id, CreateMethod> map;
    return map;
  }
};

// --

// bool factoryInit();
