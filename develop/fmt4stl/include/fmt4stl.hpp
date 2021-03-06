#pragma once

#include <type_traits>
#include <valarray>
#include <tuple>
#include <unordered_set>

#include <fmt/format.h>

// --

namespace fmt4stl {

// --

namespace detail {

// std::string and std::string_view always have substr() method.
template <typename T> class has_substr {
  template <typename U> static auto check(U *p) -> decltype(p->substr());
  template <typename> static void check(...);

public:
  static constexpr const bool value =
      !std::is_void<decltype(check<T>(nullptr))>::value;
  using type = T;
};

template <typename T> class has_const_iterator {
  template <typename U> static auto check(U *p) -> typename U::const_iterator *;
  template <typename> static void check(...);

public:
  static constexpr const bool value =
      !std::is_void<decltype(check<T>(nullptr))>::value;
  using type = T;
};

template <typename T> class has_begin_end {
  template <typename U>
  static auto f(U *p) -> decltype(p->begin(), p->cbegin());
  template <typename> static void f(...);

  template <typename U> static auto g(U *p) -> decltype(p->end(), p->cbegin());
  template <typename> static void g(...);

public:
  static constexpr const bool beg_value =
      !std::is_void<decltype(f<T>(nullptr))>::value;
  static constexpr const bool end_value =
      !std::is_void<decltype(g<T>(nullptr))>::value;
};

} // namespace detail

// --

// Basic is_container template; specialize to derive from std::true_type for all
// desired container types

template <typename T>
struct is_container
    : public std::integral_constant<bool,
                                    !detail::has_substr<T>::value &&
                                        detail::has_const_iterator<T>::value &&
                                        detail::has_begin_end<T>::beg_value &&
                                        detail::has_begin_end<T>::end_value> {};

template <typename T> struct is_container<std::valarray<T>> : std::true_type {};

// --

// Holds the delimiter values for a specific character type

template <typename TChar> struct delimiters_values {
  using char_type = TChar;
  const char_type *prefix;
  const char_type *delimiter;
  const char_type *postfix;
};

// Defines the delimiter values for a specific container and character type

template <typename T, typename TChar> struct delimiters {
  using type = delimiters_values<TChar>;
  static constexpr const type values;
};

// --

// Default delimiters

template <typename T> struct delimiters<T, char> {
  static constexpr const delimiters_values<char> values = {"[", ", ", "]"};
};
template <typename T> struct delimiters<T, wchar_t> {
  static constexpr const delimiters_values<wchar_t> values = {L"[", L", ",
                                                              L"]"};
};

// Delimiters for (multi)set and unordered_(multi)set

template <typename T, typename TComp, typename TAllocator>
struct delimiters<::std::set<T, TComp, TAllocator>, char> {
  static constexpr const delimiters_values<char> values = {"{", ", ", "}"};
};
template <typename T, typename TComp, typename TAllocator>
struct delimiters<::std::set<T, TComp, TAllocator>, wchar_t> {
  static constexpr const delimiters_values<wchar_t> values = {L"{", L", ",
                                                              L"}"};
};

template <typename T, typename TComp, typename TAllocator>
struct delimiters<::std::multiset<T, TComp, TAllocator>, char> {
  static constexpr const delimiters_values<char> values = {"{", ", ", "}"};
};
template <typename T, typename TComp, typename TAllocator>
struct delimiters<::std::multiset<T, TComp, TAllocator>, wchar_t> {
  static constexpr const delimiters_values<wchar_t> values = {L"{", L", ",
                                                              L"}"};
};

template <typename T, typename THash, typename TEqual, typename TAllocator>
struct delimiters<::std::unordered_set<T, THash, TEqual, TAllocator>, char> {
  static constexpr const delimiters_values<char> values = {"{", ", ", "}"};
};
template <typename T, typename THash, typename TEqual, typename TAllocator>
struct delimiters<::std::unordered_set<T, THash, TEqual, TAllocator>, wchar_t> {
  static constexpr const delimiters_values<wchar_t> values = {L"{", L", ",
                                                              L"}"};
};

template <typename T, typename THash, typename TEqual, typename TAllocator>
struct delimiters<::std::unordered_multiset<T, THash, TEqual, TAllocator>,
                  char> {
  static constexpr const delimiters_values<char> values = {"{", ", ", "}"};
};

template <typename T, typename THash, typename TEqual, typename TAllocator>
struct delimiters<::std::unordered_multiset<T, THash, TEqual, TAllocator>,
                  wchar_t> {
  static constexpr const delimiters_values<wchar_t> values = {L"{", L", ",
                                                              L"}"};
};

// Delimiters for pair and tuple

template <typename T1, typename T2> struct delimiters<std::pair<T1, T2>, char> {
  static constexpr const delimiters_values<char> values = {"(", ": ", ")"};
};
template <typename T1, typename T2>
struct delimiters<::std::pair<T1, T2>, wchar_t> {
  static constexpr const delimiters_values<wchar_t> values = {L"(", L": ",
                                                              L")"};
};

template <typename... Args> struct delimiters<std::tuple<Args...>, char> {
  static constexpr const delimiters_values<char> values = {"(", ", ", ")"};
};
template <typename... Args> struct delimiters<::std::tuple<Args...>, wchar_t> {
  static constexpr const delimiters_values<wchar_t> values = {L"(", L", ",
                                                              L")"};
};

} // namespace fmt4stl

// --

FMT_BEGIN_NAMESPACE
namespace internal {

template <typename RangeT, typename OutputIterator>
OutputIterator copy(const RangeT &range, OutputIterator out) {
  for (auto it = range.begin(), end = range.end(); it != end; ++it)
    *out++ = *it;
  return out;
}

template <typename OutputIterator>
OutputIterator copy(const char *str, OutputIterator out) {
  while (*str)
    *out++ = *str++;
  return out;
}

template <typename OutputIterator>
OutputIterator copy(char ch, OutputIterator out) {
  *out++ = ch;
  return out;
}

// --

/// Return true value if T has std::string interface, like std::string_view.
template <typename T> class is_like_std_string {
  template <typename U>
  static auto check(U *p)
      -> decltype((void)p->find('a'), p->length(), (void)p->data(), int());
  template <typename> static void check(...);

public:
  static FMT_CONSTEXPR_DECL const bool value =
      is_string<T>::value || !std::is_void<decltype(check<T>(nullptr))>::value;
};

template <typename Char>
struct is_like_std_string<fmt::basic_string_view<Char>> : std::true_type {};

// --

template <class Tuple, class F, size_t... Is>
void for_each(std::index_sequence<Is...>, Tuple &&tup, F &&f) FMT_NOEXCEPT {
  using std::get;
  // using free function get<I>(T) now.
  const int _[] = {0, ((void)f(get<Is>(tup)), 0)...};
  (void)_; // blocks warnings
}

template <class T>
FMT_CONSTEXPR std::
make_index_sequence<std::tuple_size<T>::value>
get_indexes(T const &) {
  return {};
}

template <class Tuple, class F> void for_each(Tuple &&tup, F &&f) {
  const auto indexes = get_indexes(tup);
  for_each(indexes, std::forward<Tuple>(tup), std::forward<F>(f));
}

// --

template <typename Arg, FMT_ENABLE_IF(!is_like_std_string<
                                      typename std::decay<Arg>::type>::value)>
FMT_CONSTEXPR const char *format_str_quoted(const Arg &) {
  return "{}";
}

template <typename Arg, FMT_ENABLE_IF(is_like_std_string<
                                      typename std::decay<Arg>::type>::value)>
FMT_CONSTEXPR const char *format_str_quoted(const Arg &) {
  return "\"{}\"";
}

FMT_CONSTEXPR const char *format_str_quoted(const char *) { return "\"{}\""; }
FMT_CONSTEXPR const wchar_t *format_str_quoted(bool add_space,
                                               const wchar_t *) {
  return L"\"{}\"";
}

FMT_CONSTEXPR const char *format_str_quoted(const char) { return "'{}'"; }
FMT_CONSTEXPR const wchar_t *format_str_quoted(const wchar_t) {
  return L"'{}'";
}

} // namespace internal

FMT_END_NAMESPACE

// --

namespace fmt {

template <typename T, typename Char>
struct formatter<T, Char, enable_if_t<fmt4stl::is_container<T>::value>> {

  using Delimiters = fmt4stl::delimiters<T, Char>;

  template <typename ParseContext> constexpr auto parse(ParseContext &ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const T &v, FormatContext &ctx) {
    std::size_t i = 0;
    auto out = ctx.out();
    internal::copy(Delimiters::values.prefix, out);
    for (const auto &e : v) {
      if (i > 0) {
        internal::copy(Delimiters::values.delimiter, out);
      }
      format_to(out, internal::format_str_quoted(e), e);
      ++i;
    }
    internal::copy(Delimiters::values.postfix, out);
    return ctx.out();
  }
};

template <typename T1, typename T2, typename Char>
struct formatter<std::pair<T1, T2>, Char> {

  using Delimiters = fmt4stl::delimiters<std::pair<T1, T2>, Char>;

  template <typename ParseContext> constexpr auto parse(ParseContext &ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const std::pair<T1, T2> &v, FormatContext &ctx) {
    auto out = ctx.out();
    internal::copy(Delimiters::values.prefix, out);
    format_to(out, internal::format_str_quoted(v.first), v.first);
    internal::copy(Delimiters::values.delimiter, out);
    format_to(out, internal::format_str_quoted(v.second), v.second);
    internal::copy(Delimiters::values.postfix, out);
    return ctx.out();
  }
};

template <typename Char, typename... Args>
struct formatter<std::tuple<Args...>, Char> {

  using Delimiters = fmt4stl::delimiters<std::tuple<Args...>, Char>;

  // C++11 generic lambda for format()
  template <typename FormatContext> struct format_each {
    template <typename T> void operator()(const T &v) {
      if (i > 0) {
        out = internal::copy(Delimiters::values.delimiter, out);
      }
      out = format_to(out, internal::format_str_quoted(v), v);
      ++i;
    }

    std::size_t &i;
    typename std::add_lvalue_reference<decltype(
        std::declval<FormatContext>().out())>::type out;
  };

public:
  template <typename ParseContext> constexpr auto parse(ParseContext &ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const std::tuple<Args...> &v, FormatContext &ctx) {
    auto out = ctx.out();
    std::size_t i = 0;
    internal::copy(Delimiters::values.prefix, out);
    internal::for_each(v, format_each<FormatContext>{i, out});
    internal::copy(Delimiters::values.postfix, out);
    return ctx.out();
  }
};

} // namespace fmt
