#include <iostream>
#include <string>
#include <vector>

#include "factory.hpp"

// --

struct MessageBase {
  using Id = std::string;
  virtual ~MessageBase() {}
  virtual void show(std::ostream &) = 0;
};

class Message1 : public MessageBase,
                 RegisterInFactory<MessageBase, Message1, std::string> {
  std::string _str;

public:
  Message1(std::string const &str) : _str(str) {}
  static Id id() { return std::string("msg1"); }

  void show(std::ostream &os) override {
    os << "Message1: _str = " << _str << std::endl;
  }
};

class Message2 : public MessageBase, RegisterInFactory<MessageBase, Message2> {
public:
  Message2() {}
  static Id id() { return std::string("msg2"); }

  void show(std::ostream &os) override { os << "Message2: " << std::endl; }
};

class Message3 : public MessageBase,
                 RegisterInFactory<MessageBase, Message3, int, double> {
  int _i;
  double _d;

public:
  Message3(int i, double d) : _i(i), _d(d) {}
  static Id id() { return std::string("msg3"); }

  void show(std::ostream &os) override {
    os << "Message3: _i = " << _i << ", _d = " << _d << std::endl;
  }
};

// --

int main(int argc, char* argv[]) {
  RegisterInFactory<MessageBase, Message1, std::string>();
  RegisterInFactory<MessageBase, Message2>();
  RegisterInFactory<MessageBase, Message3, int, double>();

  std::vector<MessageBase::Id> id {"msg1", "msg2", "msg3"};
  auto m = Factory<MessageBase, std::string>::create(id.at(0), "abc");
  m->show(std::cout);
  m = Factory<MessageBase>::create(id.at(1));
  m->show(std::cout);
  m = Factory<MessageBase, int, double>::create(id.at(2), 333, 8.8);
  m->show(std::cout);

  return EXIT_SUCCESS;
}

