#include <iostream>
#include <sstream>
#include <cassert>
#include <thread>
#include <lua.hpp>
#include "utilities.h"

// --

static std::string valueToString(lua_State* L, int idx)
{
    std::stringstream r;
    r << luaL_typename(L, idx) << ": ";

    switch (lua_type(L, idx)) {
    case LUA_TNONE:
        r << "none";
        break;

    case LUA_TNIL:
        r << "nil";
        break;

    case LUA_TBOOLEAN:
        r << std::boolalpha << lua_toboolean(L, idx);
        break;

    case LUA_TNUMBER:
        r << lua_tonumber(L, idx);
        break;

    case LUA_TSTRING:
        r << lua_tostring(L, idx);
        break;

    case LUA_TFUNCTION:
        r << lua_topointer(L, idx);
        break;

    case LUA_TUSERDATA:
        r << lua_touserdata(L, idx);
        break;

    case LUA_TLIGHTUSERDATA:
        r << lua_topointer(L, idx);
        break;

    case LUA_TTABLE:
        r << lua_topointer(L, idx);
        break;

    case LUA_TTHREAD:
        r << lua_topointer(L, idx);
        break;

    default:
        assert(false);
    }

    return r.str();
}

void stackindex_stdcout(lua_State *ls, int idx)
{
    std::cout << "[" << idx << "] = " << valueToString(ls, idx)
              << std::endl;
}

void dumpStack(const char* where,
               lua_State* L,
               void(*f)(lua_State*, int))
{
    if (where) {
        std::cout << where << std::endl;
    }
    std::cout << "----- stack dump -----" << std::endl;
    int top = lua_gettop(L);
    for (int i = top; i >= 1; --i) {
        f(L, i);
    }
    std::cout << "--------- end --------" << std::endl;
}

// --

// 返回为空，The execution of the next instruction is delayed an implementation specific
// amount of time. The instruction does not modify the architectural state. This intrinsic
// provides especially significant performance gain
extern void _mm_pause(void);

Spin_lock::Spin_lock()
{
  d_atomic_bool.store( false, std::memory_order_relaxed );
  return;
}

void Spin_lock::lock( void )
{
  while( d_atomic_bool.exchange( true, std::memory_order_acquire ) ) {
    while( 1 ) {
      _mm_pause();         // pause指令 延迟时间大约是12纳秒
      if( !d_atomic_bool.load( std::memory_order_relaxed ) ) break;
      std::this_thread::yield();         // 在无其他线程等待执行的情况下，延迟时间113纳秒
      // 在有其他线程等待执行情况下，将切换线程
      if( !d_atomic_bool.load( std::memory_order_relaxed ) ) break;
      continue;
    }
    continue;
  }
  return;
}

bool Spin_lock::try_lock( void )
{
  return !d_atomic_bool.exchange( true, std::memory_order_acquire );
}

void Spin_lock::unlock( void )
{
  d_atomic_bool.store( false, std::memory_order_release ); // 设置为false
  return;
}
