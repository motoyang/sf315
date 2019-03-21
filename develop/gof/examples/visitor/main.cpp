#include <iostream>
#include <memory>
#include <string>

#include "factory.hpp"
#include "visitor.hpp"

// --

struct animal: public Acceptor {
  using Id = std::string;

  virtual int move() = 0;
  virtual ~animal() = default;
};

struct dog : public Visitable<animal, dog> {
  static Id id() {return "dog";}
  int move() override { return 4; }
  void swim() { std::cout << "swim" << std::endl; }
  void swim2() { std::cout << "swim2" << std::endl; }
};

struct bird : public Visitable<animal, bird> {
  static Id id() {return "bird";}
  int move() override { return 2; }
  void fly() { std::cout << "fly" << std::endl; }
  void fly2() { std::cout << "fly2" << std::endl; }
};

struct fish : public Visitable<animal, fish> {
  static Id id() {return "fish";}
  int move() override { return 1; }
  void dive() { std::cout << "dive" << std::endl; }
  void dive2() { std::cout << "dive2" << std::endl; }
};

// --

struct visitor_impl : public Visitor<dog, bird, fish> {
  void visit(dog *d) override { d->swim(); }
  void visit(bird *b) override { b->fly(); }
  void visit(fish *f) override { f->dive(); }
};

struct visitor_impl2 : public Visitor<dog, bird> {
  void visit(dog *d) override { d->swim2(); }
  void visit(bird *b) override { b->fly2(); }
};

// --

int main() {
  RegisterInFactory<animal, dog>();
  RegisterInFactory<animal, bird>();
  RegisterInFactory<animal, fish>();

  auto a = Factory<animal>::create("dog");
  auto b = Factory<animal>::create("bird");
  auto c = Factory<animal>::create("fish");

  auto v = std::make_unique<visitor_impl>();
  a->accept(v.get());
  b->accept(v.get());
  c->accept(v.get());

  auto v2 = std::make_unique<visitor_impl2>();
  a->accept(v2.get());
  b->accept(v2.get());

  return 0;
}
