#include <lua.hpp>
#include <QDebug>
#include <iostream>
#include "LuaBridge_cpp17/LuaBridge.h"
#include "qcp/qcustomplot.h"
#include "utilities.h"
//#include "luaplot.h"
//#include "mainwindow.h"
#include "type4stack.h"
#include "qt2lua.h"
#include "qcp2lua.h"
#include "cpp2lua.h"

// --

QVector<double> getQVector(const QVector<double>& v)
{
    QVector<double> r(v);
    r.append(v);
    return r;
}

QString getQString(const QString& s)
{
    return s + " + " + s;
}

QCPStatisticalBoxData getStatisticalBoxData(const QCPStatisticalBoxData& d)
{
    QCPStatisticalBoxData r(d);
    r.key += 1;
    r.minimum += 2;
    r.maximum += 3;
    r.lowerQuartile += 4;
    r.median += 5;
    r.upperQuartile += 6;
    for (double& v: r.outliers) {
        v++;
    }

    return r;
}

// --

static void Tester2Lua(lua_State* L, const char* ns)
{
    luabridge::getGlobalNamespace(L)
      .beginNamespace(ns)

        .addFunction("getQVector", getQVector)
        .addFunction("getQString", getQString)
        .addFunction("getStatisticalBoxData", getStatisticalBoxData)

//        .beginClass<aaa>("aaa")
//        .endClass()

//          .deriveClass<bbb, aaa>("bbb")
//          .endClass()

      .endNamespace()
    ;
}

// --

void Cpp2Lua(lua_State* L, const char* ns)
{
    Qt2Lua(L, ns);
    Qcp2Lua(L, ns);

    Tester2Lua(L, ns);
}
