#include <iostream>

#include "visitor.hpp"

int main() {
  auto f = [](auto p) { std::cout << p << std::endl; };
  visitArgs(f, 1, 2.2f, "aaa", std::string("abc"), 2.5, 'd');

  auto t =
      std::make_tuple(1, 2L, "aaa", std::string("ccc"), 2.3, "ace", 1e-10, 'c');
  for (std::size_t i = 0; i < std::tuple_size<decltype(t)>::value; ++i) {
    visitTupleByIndex(
        i, t, [](auto p) { std::cout << p << std::endl; },
        std::make_index_sequence<std::tuple_size<decltype(t)>::value>{});
  }

  return 0;
}
