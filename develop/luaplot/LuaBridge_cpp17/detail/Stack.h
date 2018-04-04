#pragma once

//------------------------------------------------------------------------------
// Receive the lua_State* as an argument.
template <>
struct Stack <lua_State*>
{
  static lua_State* get (lua_State* L, int)
  {
    return L;
  }
};

//------------------------------------------------------------------------------
//  Push a lua_CFunction.
template <>
struct Stack <lua_CFunction>
{
  static void push (lua_State* L, lua_CFunction f)
  {
    lua_pushcfunction (L, f);
  }

  static lua_CFunction get (lua_State* L, int index)
  {
    return lua_tocfunction (L, index);
  }
};

//------------------------------------------------------------------------------
// for integral type, include unsigned char, but except bool and char.
template<typename T>
struct Stack<T, typename std::enable_if_t<std::is_integral<T>::value
        && !std::is_same<T, bool>::value && !std::is_same<T, char>::value > >
{
    static inline void push (lua_State* L, T value)
    {
      lua_pushinteger (L, static_cast <lua_Integer> (value));
    }

    static inline T get (lua_State* L, int index)
    {
      return static_cast <T> (luaL_checkinteger (L, index));
    }
};

//------------------------------------------------------------------------------
// Stack specialization for `bool`.
template <>
struct Stack <bool> {
  static inline void push (lua_State* L, bool value)
  {
    lua_pushboolean (L, value ? 1 : 0);
  }
  
  static inline bool get (lua_State* L, int index)
  {
    return lua_toboolean (L, index) ? true : false;
  }
};

//------------------------------------------------------------------------------
// Stack specialization for `char`.
template <>
struct Stack <char>
{
  static inline void push (lua_State* L, char value)
  {
    char str [2] = { value, 0 };
    lua_pushstring (L, str);
  }
  
  static inline char get (lua_State* L, int index)
  {
    return luaL_checkstring (L, index) [0];
  }
};

//------------------------------------------------------------------------------
// for float point type
template<typename T>
struct Stack<T, typename std::enable_if_t<std::is_floating_point<T>::value> >
{
    static inline void push (lua_State* L, T value)
    {
      lua_pushnumber (L, static_cast <lua_Number> (value));
    }

    static inline T get (lua_State* L, int index)
    {
      return static_cast <T> (luaL_checknumber (L, index));
    }
};

//------------------------------------------------------------------------------
// for char const*
template <>
struct Stack <char const*>
{
  static inline void push (lua_State* L, char const* str)
  {
    if (str != 0)
      lua_pushstring (L, str);
    else
      lua_pushnil (L);
  }

  static inline char const* get (lua_State* L, int index)
  {
    return lua_isnil (L, index) ? 0 : luaL_checkstring (L, index);
  }
};

//------------------------------------------------------------------------------
// Stack specialization for `std::string`.
template <>
struct Stack <std::string>
{
  static inline void push (lua_State* L, std::string const& str)
  {
    lua_pushlstring (L, str.c_str (), str.size());
  }

  static inline std::string get (lua_State* L, int index)
  {
    size_t len;
    const char *str = luaL_checklstring(L, index, &len);
    return std::string (str, len);
  }
};
