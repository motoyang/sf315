#ifndef UTILITIES_H
#define UTILITIES_H

void stackindex_stdcout(lua_State *ls, int idx);
void dumpStack(const char* where, lua_State* L, void(*f)(lua_State*, int) = stackindex_stdcout);

#endif // UTILITIES_H
