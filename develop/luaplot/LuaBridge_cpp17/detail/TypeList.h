#pragma once

#include <utility>
#include <tuple>

// --

// be similar to std::apply()
template<typename T, typename Function, typename Tuple, std::size_t... Index>
decltype(auto) obj_invoke_impl(T* obj, Function&& func, Tuple&& t, std::index_sequence<Index...>)
{
    return (obj->*func)(std::get<Index>(std::forward<Tuple>(t))...);
}

template<typename T, typename Function, typename Tuple>
decltype(auto) obj_apply(T* obj, Function&& func, Tuple&& t)
{
    constexpr auto size = std::tuple_size<typename std::decay<Tuple>::type>::value;
    return obj_invoke_impl(obj, std::forward<Function>(func), std::forward<Tuple>(t), std::make_index_sequence<size>{});
}

// --

template<int index, typename Tuple>
struct ArgList2;

template <int index, typename Head, typename... Args>
struct ArgList2<index, std::tuple<Head, Args...>>
{
    Head _h;
    ArgList2<index+1, std::tuple<Args...>> _t;

    ArgList2(lua_State* L)
        : _h(Stack<Head>::get(L, index))
        , _t(L)
    {}

    decltype(auto) tuple() const
    {
        return std::tuple_cat(std::make_tuple(_h), _t.tuple());
    }
};

template <int index, typename Head, typename... Args>
struct ArgList2<index, std::tuple<Head&, Args...>>
    : public ArgList2<index, std::tuple<Head, Args...>>
{
    ArgList2(lua_State* L)
        : ArgList2<index, std::tuple<Head, Args...>>(L)
    {}
};

template <int index, typename Head, typename... Args>
struct ArgList2<index, std::tuple<Head const&, Args...>>
    : public ArgList2<index, std::tuple<Head, Args...>>
{
    ArgList2(lua_State* L)
        : ArgList2<index, std::tuple<Head, Args...>>(L)
    {}
};

template <int index, typename Head>
struct ArgList2<index, std::tuple<Head>>
{
    Head _h;
    ArgList2(lua_State* L)
        : _h(Stack<Head>::get(L, index))
    {
    }

    decltype(auto) tuple() const
    {
        return std::make_tuple(_h);
    }
};

template <int index, typename Head>
struct ArgList2<index, std::tuple<Head&>>
    : public ArgList2<index, std::tuple<Head>>
{
    ArgList2(lua_State* L)
        : ArgList2<index, std::tuple<Head>>(L)
    {}
};

template <int index, typename Head>
struct ArgList2<index, std::tuple<Head const&>>
    : public ArgList2<index, std::tuple<Head>>
{
    ArgList2(lua_State* L)
        : ArgList2<index, std::tuple<Head>>(L)
    {}
};

// 参数列表为空
template <int index>
struct ArgList2<index, std::tuple<>>
{
    ArgList2(lua_State* L)
    {
    }

    decltype(auto) tuple() const
    {
        return std::tuple<>();
    }
};

