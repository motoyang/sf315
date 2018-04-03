//------------------------------------------------------------------------------
/*
  https://github.com/vinniefalco/LuaBridge
  
  Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>

  License: The MIT License (http://www.opensource.org/licenses/mit-license.php)

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/
//==============================================================================

/**
  Since the throw specification is part of a function signature, the FuncTraits
  family of templates needs to be specialized for both types. The
  LUABRIDGE_THROWSPEC macro controls whether we use the 'throw ()' form, or
  'noexcept' (if C++11 is available) to distinguish the functions.
*/
#if defined (__APPLE_CPP__) || defined(__APPLE_CC__) || defined(__clang__) || defined(__GNUC__) || \
    (defined (_MSC_VER) && (_MSC_VER >= 1700))
// Do not define LUABRIDGE_THROWSPEC since the Xcode and gcc  compilers do not
// distinguish the throw specification in the function signature.
#else
// Visual Studio 10 and earlier pay too much mind to useless throw() spec.
//
# define LUABRIDGE_THROWSPEC throw()
#endif

//==============================================================================
/**
    Traits for function pointers.

    There are three types of functions: global, non-const member, and const
    member. These templates determine the type of function, which class type it
    belongs to if it is a class member, the const-ness if it is a member
    function, and the type information for the return value and argument list.

    Expansions are provided for functions with up to 8 parameters. This can be
    manually extended, or expanded to an arbitrary amount using C++11 features.
*/

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
    static R call (D fp, ArgList2<1, Params> tvl)
    {
        return std::apply(fp, tvl.tuple());
    }
};

template<typename R, typename D, typename... Args>
struct FuncTraits2<R(*)(Args...) noexcept, D>
    : public FuncTraits2<R(*)(Args...), D>
{};

#ifdef LUABRIDGE_THROWSPEC
template<typename R, typename D, typename... Args>
struct FuncTraits2<R(*)(Args...) LUABRIDGE_THROWSPEC, D>
    : public FuncTraits2<R(*)(Args...), D>
{};
#endif

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
    static R call (T* obj, D fp, ArgList2<2, Params>& tvl)
    {
        return obj_apply(obj, fp, tvl.tuple());
    }
};

template<typename T, typename R, typename D, typename... Args>
struct FuncTraits2<R(T::*)(Args...) noexcept, D>
    : public FuncTraits2<R(T::*)(Args...), D>
{};

#ifdef LUABRIDGE_THROWSPEC
template<typename T, typename R, typename D, typename... Args>
struct FuncTraits2<R(T::*)(Args...) LUABRIDGE_THROWSPEC, D>
    : public FuncTraits2<R(T::*)(Args...), D>
{};
#endif

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
    static R call (T const* obj, D fp, ArgList2<2, Params> tvl)
    {
        return obj_apply(obj, fp, tvl.tuple());
    }
};

template<typename T, typename R, typename D, typename... Args>
struct FuncTraits2<R(T::*)(Args...)const noexcept, D>
    : public FuncTraits2<R(T::*)(Args...)const, D>
{};

#ifdef LUABRIDGE_THROWSPEC
template<typename T, typename R, typename D, typename... Args>
struct FuncTraits2<R(T::*)(Args...)const LUABRIDGE_THROWSPEC, D>
    : public FuncTraits2<R(T::*)(Args...) const, D>
{};
#endif

// -- end of FuncTraits2

