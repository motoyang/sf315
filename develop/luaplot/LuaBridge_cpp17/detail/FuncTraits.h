#pragma once

// -- begin of FuncTraits2

// this is general template
template <class MemFn, class D = MemFn>
struct FuncTraits2;

// for global and static member functions
template<typename R, typename D, typename... Args>
struct FuncTraits2<R(*)(Args...), D>
{
    static bool const isMemberFunction = false;
    typedef D DeclType;
    typedef R ReturnType;
    typedef std::tuple<Args...> Params;
    static R call (D fp, const ArgList2<1, Params>& tvl)
    {
        return std::apply(fp, tvl.tuple());
    }
};

template<typename R, typename D, typename... Args>
struct FuncTraits2<R(*)(Args...) noexcept, D>
    : public FuncTraits2<R(*)(Args...), D>
{};

// for member functions
template<typename T, typename R, typename D, typename... Args>
struct FuncTraits2<R(T::*)(Args...), D>
{
    static bool const isMemberFunction = true;
    static bool const isConstMemberFunction = false;
    typedef D DeclType;
    typedef T ClassType;
    typedef R ReturnType;
    typedef std::tuple<Args...> Params;
    static R call (T* obj, D fp, const ArgList2<2, Params>& tvl)
    {
        return obj_apply(obj, fp, tvl.tuple());
    }
};

template<typename T, typename R, typename D, typename... Args>
struct FuncTraits2<R(T::*)(Args...) noexcept, D>
    : public FuncTraits2<R(T::*)(Args...), D>
{};

// for const member functions
template<typename T, typename R, typename D, typename... Args>
struct FuncTraits2<R(T::*)(Args...)const, D>
{
    static bool const isMemberFunction = true;
    static bool const isConstMemberFunction = true;
    typedef D DeclType;
    typedef T ClassType;
    typedef R ReturnType;
    typedef std::tuple<Args...> Params;
    static R call (T const* obj, D fp, const ArgList2<2, Params>& tvl)
    {
        return obj_apply(obj, fp, tvl.tuple());
    }
};

template<typename T, typename R, typename D, typename... Args>
struct FuncTraits2<R(T::*)(Args...)const noexcept, D>
    : public FuncTraits2<R(T::*)(Args...)const, D>
{};

// -- end of FuncTraits2

