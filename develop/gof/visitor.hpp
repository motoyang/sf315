#pragma once

// --

struct VisitorToken {
  virtual ~VisitorToken() = default;
};

template <typename T> struct Visitant { virtual void visit(T *) = 0; };

// --

struct Acceptor{
  virtual void accept(VisitorToken *) = 0;
};

template <typename Base, typename T> struct Visitable : public Base {
  void accept(VisitorToken *v) override {
    if (auto p = dynamic_cast<Visitant<T> *>(v); p) {
      p->visit(static_cast<T *>(this));
    }
  }
};

template <class... T>
struct Visitor : public VisitorToken, public Visitant<T>... {};
