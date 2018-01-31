#ifndef TYPE4STACK_H
#define TYPE4STACK_H

// --

namespace luabridge {

#define STACK_AS_INT(X) \
template <> struct Stack <X> { \
    static void push (lua_State* L, X v) { \
    lua_pushinteger(L, static_cast<int>(v)); \
    } \
    static X get (lua_State* L, int index) { \
    return static_cast<X>(luaL_checkinteger(L, index)); \
    } \
};

#define STACK_AS_INT_FROM(Y, X) \
template <> struct Stack <Y> { \
    static void push (lua_State* L, Y v) { \
    lua_pushinteger(L, static_cast<int>(v)); \
    } \
    static Y get (lua_State* L, int index) { \
    return Y(static_cast<X>(luaL_checkinteger(L, index))); \
    } \
};

#define STACK_AS_INT_CONST_FROM(Y, X) \
template <> struct Stack <Y const&> { \
    static void push (lua_State* L, Y v) { \
    lua_pushinteger(L, static_cast<int>(v)); \
    } \
    static Y get (lua_State* L, int index) { \
    return Y(static_cast<X>(luaL_checkinteger(L, index))); \
    } \
};


STACK_AS_INT(Qt::AlignmentFlag)
STACK_AS_INT(Qt::AspectRatioMode)
STACK_AS_INT(Qt::BrushStyle)
STACK_AS_INT(Qt::ConnectionType)
STACK_AS_INT(Qt::GlobalColor)
STACK_AS_INT(Qt::PenCapStyle)
STACK_AS_INT(Qt::PenJoinStyle)
STACK_AS_INT(Qt::PenStyle)
STACK_AS_INT(Qt::TimeSpec)
STACK_AS_INT(Qt::TransformationMode)
STACK_AS_INT(Qt::WidgetAttribute)
STACK_AS_INT(QLocale::Country)
STACK_AS_INT(QLocale::Language)
STACK_AS_INT(QLocale::Script)

STACK_AS_INT(QCPAxis::AxisType)
STACK_AS_INT(QCPAxis::ScaleType)
STACK_AS_INT(QCPAxisTickerFixed::ScaleStrategy)
STACK_AS_INT(QCPColorGradient::GradientPreset)
STACK_AS_INT(QCPFinancial::ChartStyle)
STACK_AS_INT(QCPGraph::LineStyle)
STACK_AS_INT(QCPLineEnding::EndingStyle)
STACK_AS_INT(QCPItemPosition::PositionType)
STACK_AS_INT(QCPItemTracer::TracerStyle)
STACK_AS_INT(QCPScatterStyle::ScatterShape)
STACK_AS_INT(QCustomPlot::LayerInsertMode)
STACK_AS_INT(QCustomPlot::RefreshPriority)

STACK_AS_INT_FROM(Qt::Alignment, Qt::AlignmentFlag)
STACK_AS_INT_FROM(Qt::ImageConversionFlags, Qt::ImageConversionFlag)

STACK_AS_INT_FROM(QCP::MarginSides, QCP::MarginSide)
STACK_AS_INT_FROM(QCPAxis::AxisTypes, QCPAxis::AxisType)

STACK_AS_INT_CONST_FROM(QCP::Interactions, QCP::Interaction)

#undef STACK_AS_INT
#undef STACK_AS_INT_FROM
#undef STACK_AS_INT_CONST_FROM

// --

template <>
struct Stack <QString>
{
    static void push (lua_State* L, QString s)
    {
        lua_pushstring (L, s.toUtf8().constData());
    }
    static QString get (lua_State* L, int index)
    {
        return QString::fromUtf8(luaL_checkstring (L, index));
    }
};

template <>
struct Stack <QString const&>
{
    static void push (lua_State* L, QString const& s)
    {
        lua_pushstring (L, s.toUtf8().constData());
    }

    static QString get (lua_State* L, int index)
    {
        return QString::fromUtf8(luaL_checkstring (L, index));
    }
};

template <typename T>
struct Stack <QSharedPointer<T>>
{
    static void push (lua_State* L, QSharedPointer<T> s)
    {
        Stack<T*>::push(L, s.data());
    }
    static QSharedPointer<T> get (lua_State* L, int index)
    {
        return QSharedPointer<T>(Stack<T*>::get(L, index));
    }
};

template <typename T>
struct Stack <QList<T*> >
{
    static void push (lua_State* L, QList<T*> const& v)
    {
        int i = 1;
        lua_newtable(L);
        for(QCPAxis* value: v) {
            Stack<T*>::push(L, value);
            lua_rawseti(L, -2, i++);
        }
    }
    static QList<T*> get (lua_State* L, int index)
    {
        Q_ASSERT(lua_istable(L, index));

        int size = lua_rawlen(L, index);
        QList<T*> v;
        for (int i = 0; i < size; ++i) {
            lua_rawgeti(L, index, i + 1);
            v.append(Stack<T*>::get(L, -1));
            lua_pop(L, 1);
        }

        return v;
    }
};

template <typename T>
struct Stack <QVector<T> const&>
{
    static void push (lua_State* L, QVector<T> const& v)
    {
//        dumpStack(Q_FUNC_INFO, L);

        lua_newtable(L);
        int i = 1;
        for(T value: v)
        {
            Stack<T>::push(L, value);
            lua_rawseti(L, -2, i++);       //push key,value
        }
//        dumpStack(nullptr, L);
    }

    static QVector<T> get (lua_State* L, int index)
    {
//        dumpStack(Q_FUNC_INFO, L);
        Q_ASSERT(lua_istable(L, index));

        int size = lua_rawlen(L, index);
        QVector<T> v(size);
        for (int i = 0; i < size; ++i) {
            lua_rawgeti(L, index, i + 1);
            v[i] = Stack<T>::get(L, -1);
            lua_pop(L, 1);
        }

//        dumpStack(nullptr, L);
        return v;
    }
};

template <typename T>
struct Stack <QVector<T> >
{
    static void push (lua_State* L, QVector<T> const& v)
    {
        Stack<QVector<T> const&>::push(L, v);
    }

    static QVector<T> get (lua_State* L, int index)
    {
        return Stack<QVector<T> const&>::get(L, index);
    }
};

template <>
struct Stack <QCPGraphData const&>
{
    static const char* key;
    static const char* value;
    static void push (lua_State* L, QCPGraphData const& v)
    {
//        dumpStack(Q_FUNC_INFO, L);

        lua_newtable(L);
        lua_pushstring(L, key);
        lua_pushnumber(L, v.key);
        lua_rawset(L, -2);       //push key,value
        lua_pushstring(L, value);
        lua_pushnumber(L, v.value);
        lua_rawset(L, -2);       //push key,value

//        dumpStack(nullptr, L);
    }

    static QCPGraphData get (lua_State* L, int index)
    {
//        dumpStack(Q_FUNC_INFO, L);
        Q_ASSERT(lua_istable(L, index));

        QCPGraphData d;
        lua_pushstring(L, key);
        lua_rawget(L, index-1);
        d.key = lua_tonumber(L, -1);
        lua_pop(L, 1);
        lua_pushstring(L, value);
        lua_rawget(L, index-1);
        d.value = lua_tonumber(L, -1);
        lua_pop(L, 1);

//        dumpStack(nullptr, L);
        return d;
    }
};
const char* Stack <QCPGraphData const&>::key = "key";
const char* Stack <QCPGraphData const&>::value = "value";

template <>
struct Stack <QCPGraphData>
{
    static void push (lua_State* L, QCPGraphData const& v)
    {
        Stack<QCPGraphData const&>::push(L, v);
    }

    static QCPGraphData get (lua_State* L, int index)
    {
        return Stack<QCPGraphData const&>::get(L, index);
    }
};

template <>
struct Stack <QCPCurveData const&>
{
    static const char* t;
    static const char* key;
    static const char* value;
    static void push (lua_State* L, QCPCurveData const& v)
    {
//        dumpStack(Q_FUNC_INFO, L);

        lua_newtable(L);
        lua_pushstring(L, t);
        lua_pushnumber(L, v.t);
        lua_rawset(L, -2);       //push key,value
        lua_pushstring(L, key);
        lua_pushnumber(L, v.key);
        lua_rawset(L, -2);       //push key,value
        lua_pushstring(L, value);
        lua_pushnumber(L, v.value);
        lua_rawset(L, -2);       //push key,value

//        dumpStack(nullptr, L);
    }

    static QCPCurveData get (lua_State* L, int index)
    {
//        dumpStack(Q_FUNC_INFO, L);
        Q_ASSERT(lua_istable(L, index));

        QCPCurveData d;
        lua_pushstring(L, t);
        lua_rawget(L, index-1);
        d.t = lua_tonumber(L, -1);
        lua_pop(L, 1);
        lua_pushstring(L, key);
        lua_rawget(L, index-1);
        d.key = lua_tonumber(L, -1);
        lua_pop(L, 1);
        lua_pushstring(L, value);
        lua_rawget(L, index-1);
        d.value = lua_tonumber(L, -1);
        lua_pop(L, 1);

//        dumpStack(nullptr, L);
        return d;
    }
};
const char* Stack <QCPCurveData const&>::t = "t";
const char* Stack <QCPCurveData const&>::key = "key";
const char* Stack <QCPCurveData const&>::value = "value";

template <>
struct Stack <QCPCurveData>
{
    static void push (lua_State* L, QCPCurveData const& v)
    {
        Stack<QCPCurveData const&>::push(L, v);
    }

    static QCPCurveData get (lua_State* L, int index)
    {
        return Stack<QCPCurveData const&>::get(L, index);
    }
};

}   // namespace luabridge end.

#endif // TYPE4STACK_H
