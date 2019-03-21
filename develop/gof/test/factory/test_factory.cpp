#include <iostream>
#include <memory>

#include <factory.hpp>

#include "doctest/doctest.h"

struct Base {
  using Id = int;
  virtual ~Base() = default;
  virtual int test(int i) = 0;
};

template <size_t I> struct Drived : public Base {
  static Id id() { return I; }
  int test(int i) override { return i + id(); }
};

struct Base2 {
  using Id = std::string;
  virtual ~Base2() = default;
  virtual std::string test(const std::string &s) = 0;
};

struct Drived2 : public Base2 {
  static Id id() { return "D2"; }
  std::string test(const std::string &str) override {
    return str + " - " + id();
  }
};

template <size_t I>
std::ostream &operator<<(std::ostream &os,
                         const std::unique_ptr<Drived<I>> &value) {
  os << "Drived<" << I << ">";
  return os;
}

std::ostream &operator<<(std::ostream &os, const std::unique_ptr<Base> &value) {
  os << "std::unique_ptr<Base>";
  return os;
}

std::ostream &operator<<(std::ostream &os, const std::unique_ptr<Base2> &value) {
  os << "std::unique_ptr<Base2>";
  return os;
}

TEST_CASE("testing the factory of int id") {
  CHECK(Factory<Base>::create(1) == nullptr);

  RegisterInFactory<Base, Drived<1>>();
  auto d = Factory<Base>::create(1);
  CHECK(d);
  CHECK(d->test(2) == 3);

  RegisterInFactory<Base, Drived<33>>();
  d = Factory<Base>::create(33);
  CHECK(d);
  CHECK(d->test(22) == 55);
}

TEST_CASE("testing the factory of string id") {
  CHECK_FALSE(Factory<Base2>::create("D2"));

  RegisterInFactory<Base2, Drived2>();
  auto d2 = Factory<Base2>::create("D2");
  CHECK(d2);
  CHECK(d2->test("abc") == "abc - D2");
}
