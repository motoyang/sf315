#pragma once

// --

void Qt2Lua(lua_State* L, const char* nameSpace);

// --

template<typename T>
struct Builder
{
    template<typename... Args>
    static T from(Args... args)
    {
        return T(std::forward<Args>(args)...);
    }
};
