#pragma once

// --

struct VisitorToken {
  virtual ~VisitorToken() = default;
};

template <typename T> struct Visitant { virtual void visit(T *) = 0; };

// --

struct Acceptor {
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

// --

template <typename F, typename... Args> void visitArgs(F &&f, Args &&... args) {
  int arr[] = {(std::forward<F>(f)(std::forward<Args>(args)), 0)...};
}

#include <tuple>
#include <utility>

template <class Tuple, class F, std::size_t... Idx>
void visitTupleByIndex(
    std::size_t i, Tuple &&t, F &&f,
    std::index_sequence<Idx...> =
        std::make_index_sequence<std::tuple_size<Tuple>::value>{}) {
  bool arr[] = {
      (i == Idx && ((std::forward<F>(f)(std::get<Idx>(std::forward<Tuple>(t)))),
                    false))...};
  // std::initializer_list<bool> a{
  //   (i == Idx &&
  //   ((std::forward<F>(f)(std::get<Idx>(std::forward<Tuple>(t)))),
  //   false))...};
}
