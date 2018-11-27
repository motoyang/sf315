#pragma once

#include <stack>
#include <tuple>
#include <utility>
#include <sstream>

#include <msgpack-c/msgpack.hpp>

// --

template <typename T, typename Function, typename Tuple, std::size_t... Index>
decltype(auto) obj_invoke_impl(T *obj, Function &&func, Tuple &&t,
                               std::index_sequence<Index...>) {
                                //  return obj->f33(92);
  auto f = std::mem_fn(std::forward<Function>(func));
  return f(obj, std::get<Index>(std::forward<Tuple>(t))...);
  return (obj->*func)(std::get<Index>(std::forward<Tuple>(t))...);
}

// be similar to std::apply()
template <typename T, typename Function, typename Tuple>
decltype(auto) obj_apply(T *obj, Function &&func, Tuple &&t) {
  constexpr auto size =
      std::tuple_size<typename std::decay<Tuple>::type>::value;
  return obj_invoke_impl(obj, std::forward<Function>(func),
                         std::forward<Tuple>(t),
                         std::make_index_sequence<size>{});
}

// --

template <typename Tuple> struct ArgList3;

template <typename Head, typename... Args>
struct ArgList3<std::tuple<Head, Args...>> {
  Head _h;

  ArgList3<std::tuple<Args...>> _t;

  ArgList3(std::stack<msgpack::object_handle> &soh) : _t(soh) {
    soh.top().get().convert(_h);
    soh.pop();
  }

  decltype(auto) tuple() const {
    return std::tuple_cat(std::make_tuple(_h), _t.tuple());
  }
};

template <typename Head, typename... Args>
struct ArgList3<std::tuple<Head &, Args...>>
    : public ArgList3<std::tuple<Head, Args...>> {
  ArgList3(std::stack<msgpack::object_handle> &soh)
      : ArgList3<std::tuple<Head, Args...>>(soh) {}
};

template <typename Head, typename... Args>
struct ArgList3<std::tuple<Head const &, Args...>>
    : public ArgList3<std::tuple<Head, Args...>> {
  ArgList3(std::stack<msgpack::object_handle> &soh)
      : ArgList3<std::tuple<Head, Args...>>(soh) {}
};

template <typename Head> struct ArgList3<std::tuple<Head>> {
  Head _h;

  ArgList3(std::stack<msgpack::object_handle> &soh) {
    soh.top().get().convert(_h);
    soh.pop();
  }

  decltype(auto) tuple() const { return std::make_tuple(_h); }
};

template <typename Head>
struct ArgList3<std::tuple<Head &>> : public ArgList3<std::tuple<Head>> {
  ArgList3(std::stack<msgpack::object_handle> &soh)
      : ArgList3<std::tuple<Head>>(soh) {}
};

template <typename Head>
struct ArgList3<std::tuple<Head const &>> : public ArgList3<std::tuple<Head>> {
  ArgList3(std::stack<msgpack::object_handle> &soh)
      : ArgList3<std::tuple<Head>>(soh) {}
};

// 参数列表为空
template <> struct ArgList3<std::tuple<>> {
  ArgList3(std::stack<msgpack::object_handle> &soh) {}

  decltype(auto) tuple() const { return std::tuple<>(); }
};

// -- end of ArgList3

// -- begin of FuncTraits3

// this is general template
template <class MemFn, class D = MemFn> struct FuncTraits3;

// for global and static member functions
template <typename R, typename D, typename... Args>
struct FuncTraits3<R (*)(Args...), D> {
  static bool const isMemberFunction = false;
  typedef D DeclType;
  typedef R ReturnType;
  typedef std::tuple<Args...> Params;
  static R call(D fp, const ArgList3<Params> &tvl) {
    return std::apply(fp, tvl.tuple());
  }
};

template <typename R, typename D, typename... Args>
struct FuncTraits3<R (*)(Args...) noexcept, D>
    : public FuncTraits3<R (*)(Args...), D> {};

// for member functions
template <typename T, typename R, typename D, typename... Args>
struct FuncTraits3<R (T::*)(Args...), D> {
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef std::tuple<Args...> Params;
  static R call(T *obj, D fp, const ArgList3<Params> &tvl) {
    return obj_apply(obj, fp, tvl.tuple());
  }
};

template <typename T, typename R, typename D, typename... Args>
struct FuncTraits3<R (T::*)(Args...) noexcept, D>
    : public FuncTraits3<R (T::*)(Args...), D> {};

// for const member functions
template <typename T, typename R, typename D, typename... Args>
struct FuncTraits3<R (T::*)(Args...) const, D> {
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef std::tuple<Args...> Params;
  static R call(T const *obj, D fp, const ArgList3<Params> &tvl) {
    return obj_apply(obj, fp, tvl.tuple());
  }
};

template <typename T, typename R, typename D, typename... Args>
struct FuncTraits3<R (T::*)(Args...) const noexcept, D>
    : public FuncTraits3<R (T::*)(Args...) const, D> {};

// -- end of FuncTraits3

// -- begin of Call3

template <class FnPtr,
          class ReturnType = typename FuncTraits3<FnPtr>::ReturnType>
struct Call3 {
  typedef typename FuncTraits3<FnPtr>::Params Params;

  static std::string f(std::ptrdiff_t pf, const char *buffer, std::size_t len,
                       std::size_t off, std::size_t o) {
    FnPtr const &fnptr = reinterpret_cast<FnPtr>(pf);

    std::stack<msgpack::object_handle> soh;
    while (off != len) {
      soh.push(msgpack::unpack(buffer, len, off));
    }

    std::stringstream ss;
    ArgList3<Params> args(soh);
    msgpack::pack(ss, FuncTraits3<FnPtr>::call(fnptr, args));

    return ss.str();
  }
};

// call a function with no return value
template <class FnPtr> struct Call3<FnPtr, void> {
  typedef typename FuncTraits3<FnPtr>::Params Params;
  static std::string f(std::ptrdiff_t pf, const char *buffer, std::size_t len,
                       std::size_t off, std::size_t o) {

    FnPtr const &fnptr = reinterpret_cast<FnPtr const>(pf);

    std::stack<msgpack::object_handle> soh;
    while (off != len) {
      soh.push(msgpack::unpack(buffer, len, off));
    }

    ArgList3<Params> args(soh);
    FuncTraits3<FnPtr>::call(fnptr, args);

    return std::string();
  }
};

template <class MemFnPtr,
          class ReturnType = typename FuncTraits3<MemFnPtr>::ReturnType>
struct CallMember3 {
  typedef typename FuncTraits3<MemFnPtr>::ClassType T;
  typedef typename FuncTraits3<MemFnPtr>::Params Params;

  static std::string f(std::ptrdiff_t pf, const char *buffer, std::size_t len,
                       std::size_t off, std::size_t o) {
    MemFnPtr const fnptr = *(MemFnPtr const *)pf;

    std::intptr_t oid;
    msgpack::object_handle oh = msgpack::unpack(buffer, len, off);
    oh.get().convert(oid);
    T *obj = (T *)oid;

    std::stack<msgpack::object_handle> soh;
    while (off != len) {
      soh.push(msgpack::unpack(buffer, len, off));
    }

    std::stringstream ss;
    ArgList3<Params> args(soh);
    msgpack::pack(ss, FuncTraits3<MemFnPtr>::call(obj, fnptr, args));

    return ss.str();
  }
};

template <class MemFnPtr> struct CallMember3<MemFnPtr, void> {
  typedef typename FuncTraits3<MemFnPtr>::ClassType T;
  typedef typename FuncTraits3<MemFnPtr>::Params Params;

  static std::string f(std::ptrdiff_t pf, const char *buffer, std::size_t len,
                       std::size_t off, std::size_t o) {
    MemFnPtr const fnptr = *(MemFnPtr const *)pf;

    std::intptr_t oid;
    msgpack::object_handle oh = msgpack::unpack(buffer, len, off);
    oh.get().convert(oid);
    T *obj = (T *)oid;

    std::stack<msgpack::object_handle> soh;
    while (off != len) {
      soh.push(msgpack::unpack(buffer, len, off));
    }

    ArgList3<Params> args(soh);
    FuncTraits3<MemFnPtr>::call(obj, fnptr, args);

    return std::string();
  }
};

// -- end of Call3
