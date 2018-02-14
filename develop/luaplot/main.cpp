#include <iostream>
#include <lua.hpp>
#include <QApplication>
#include "mainwindow.h"
#include "expressionwidget.h"
#include "cpp2lua.h"

// --

static bool DoLuaFile(lua_State* L,
               const QString& file,
               QString* error = nullptr)
{
    bool r = true;
    if (luaL_dofile(L, file.toUtf8().constData()) != LUA_OK) {
        if (error) {
            // 从栈顶获取错误消息。
            if (lua_gettop(L) != 0) {
                *error = QString::fromUtf8(lua_tostring(L, -1));
            }
        }
        r = false;
    }

//    lua_gc(L, LUA_GCCOLLECT, 0);
    return r;
}

static lua_State* openLua()
{
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    luaL_checkversion(L);

    return L;
}

int main(int argc, char *argv[])
{
    // 检查是否有lua文件作为参数传入，如果没有，给出usage后退出程序
    if (argc <= 1) {
        std::cout << "Usage: " << std::endl
                  << argv[0] << " filename1.lua filename2.lua ..."
                  << std::endl;

        return 0;
    }

    // QApp start
    QApplication a(argc, argv);

    // Open Lua and register cpp functions and class to Lua
    lua_State* L = openLua();
    Cpp2Lua(L, a.applicationName().toUtf8().constData());

    // Do these lua files
    for (int i = 1; i < argc; ++i) {
        QString error;
        if (!DoLuaFile(L, argv[i], &error)) {
            std::cerr << error.toUtf8().constData() << std::endl;
        }
    }

//    MainWindow w;
//    ExpressionWidget w;
//    w.resize(800, 600);
//    w.show();
//    int r = a.exec();

    lua_close(L);
    return 0;
}

