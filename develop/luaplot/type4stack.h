#ifndef TYPE4STACK_H
#define TYPE4STACK_H

// --

namespace luabridge {

// 针对enum类型的stack
template<typename T>
struct Stack<T, typename std::enable_if_t<std::is_enum<T>::value>>
{
    static void push (lua_State* L, T v) {
        lua_pushinteger(L, static_cast<int>(v));
    }
    static T get (lua_State* L, int index) {
        return static_cast<T>(luaL_checkinteger(L, index));
    }
};

// 针对QFlags<enum>类型的stack
template<typename T>
struct Stack<T, typename std::enable_if_t<
    std::is_same<T, QFlags<typename T::enum_type>>::value &&
        std::is_enum<typename T::enum_type>::value>>
{
    static void push (lua_State* L, T v) {
        Stack<typename T::Int>::push(L, v);
    }
    static T get (lua_State* L, int index) {
        return T(Stack<typename T::enum_type>::get(L, index));
    }
};

// 针对QFlags<enum> const&类型的stack
template<typename T>
struct Stack<T const&, typename std::enable_if_t<
    std::is_same<T, QFlags<typename T::enum_type>>::value &&
        std::is_enum<typename T::enum_type>::value>>
    : public Stack<T, typename std::enable_if_t<
        std::is_same<T, QFlags<typename T::enum_type>>::value &&
            std::is_enum<typename T::enum_type>::value>>
{};

// --

// lua table的get和push
struct TablePushAndGet
{
    template<typename T>
    static T getValueByName(lua_State* L, int index, const char* name)
    {
        lua_pushstring(L, name);
        lua_rawget(L, index);
        T r = Stack<T>::get(L, lua_absindex(L, -1));
        lua_pop(L, 1);
        return r;
    }

    template<typename T>
    static void pushValueByName(lua_State* L, int index, const char* name, const T& value)
    {
        lua_pushstring(L, name);
        Stack<T>::push(L, value);
        lua_rawset(L, index);
    }
};

// --

template <>
struct Stack<QString const&>
{
    static void push (lua_State* L, QString const& s)
    {
        Stack<const char*>::push(L, s.toUtf8().constData());
    }

    static QString get (lua_State* L, int index)
    {
        return QString::fromUtf8(Stack<const char*>::get(L, index));
    }
};

template <>
struct Stack<QString>
    : public Stack<QString const&>
{};

// --

template <typename T>
struct Stack<QSharedPointer<T> const&>
{
    static void push (lua_State* L, QSharedPointer<T> const& s)
    {
        Stack<T*>::push(L, s.data());
    }
    static QSharedPointer<T> get (lua_State* L, int index)
    {
        return QSharedPointer<T>(Stack<T*>::get(L, index));
    }
};

template <typename T>
struct Stack <QSharedPointer<T>>
    : public Stack<QSharedPointer<T> const&>
{};

// --

template <typename T>
struct Stack<QList<T*> const&>
{
    static void push (lua_State* L, QList<T*> const& v)
    {
        int i = 1;
        lua_newtable(L);
        for(T* value: v) {
            Stack<T*>::push(L, value);
            lua_rawseti(L, -2, i++);
        }
    }
    static QList<T*> get (lua_State* L, int index)
    {
        Q_ASSERT(lua_istable(L, index));

        QList<T*> v;
        int size = lua_rawlen(L, index);
        for (int i = 0; i < size; ++i) {
            lua_rawgeti(L, index, i + 1);
            v.append(Stack<T*>::get(L, lua_absindex(L, -1)));
            lua_pop(L, 1);
        }

        return v;
    }
};

template <typename T>
struct Stack<QList<T*>>
    : public Stack <QList<T*> const&>
{};

// --

template <typename T>
struct Stack<QVector<T> const&>
{
    static void push (lua_State* L, QVector<T> const& v)
    {
        lua_newtable(L);
        int i = 1;
        for(T value: v)
        {
            Stack<T>::push(L, value);
            lua_rawseti(L, -2, i++);       //push key,value
        }
    }

    static QVector<T> get (lua_State* L, int index)
    {
        Q_ASSERT(lua_istable(L, index));

        int size = lua_rawlen(L, index);
        QVector<T> v(size);
        for (int i = 0; i < size; ++i) {
            lua_rawgeti(L, index, i + 1);
            v[i] = Stack<T>::get(L, lua_absindex(L, -1));
            lua_pop(L, 1);
        }

        return v;
    }
};

template <typename T>
struct Stack<QVector<T>>
    : public Stack<QVector<T> const&>
{};

// --

template <>
struct Stack<QCPBarsData const&>
    : public TablePushAndGet
{
    static const char* key;
    static const char* value;

    static void push (lua_State* L, QCPBarsData const& v)
    {
        lua_newtable(L);
        int index = lua_absindex(L, -1);
        pushValueByName(L, index, key, v.key);
        pushValueByName(L, index, value, v.value);
    }

    static QCPBarsData get (lua_State* L, int index)
    {
        Q_ASSERT(lua_istable(L, index));

        QCPBarsData d;
        d.key = getValueByName<double>(L, index, key);
        d.value = getValueByName<double>(L, index, value);

        return d;
    }
};
const char* Stack<QCPBarsData const&>::key = "key";
const char* Stack<QCPBarsData const&>::value = "value";

template <>
struct Stack<QCPBarsData>
    : public Stack<QCPBarsData const&>
{};

template <>
struct Stack<QCPErrorBarsData const&>
    : public TablePushAndGet
{
    static const char* errorMinus;
    static const char* errorPlus;

    static void push (lua_State* L, QCPErrorBarsData const& v)
    {
        lua_newtable(L);
        int index = lua_absindex(L, -1);
        pushValueByName(L, index, errorMinus, v.errorMinus);
        pushValueByName(L, index, errorPlus, v.errorPlus);
    }

    static QCPErrorBarsData get (lua_State* L, int index)
    {
        Q_ASSERT(lua_istable(L, index));

        QCPErrorBarsData d;
        d.errorMinus = getValueByName<double>(L, index, errorMinus);
        d.errorPlus = getValueByName<double>(L, index, errorPlus);

        return d;
    }
};
const char* Stack<QCPErrorBarsData const&>::errorMinus = "errorMinus";
const char* Stack<QCPErrorBarsData const&>::errorPlus = "errorPlus";

template <>
struct Stack<QCPErrorBarsData>
    : public Stack<QCPErrorBarsData const&>
{};

template <>
struct Stack<QCPFinancialData const&>
    : public TablePushAndGet
{
    static const char* key;
    static const char* open;
    static const char* high;
    static const char* low;
    static const char* close;

    static void push (lua_State* L, QCPFinancialData const& v)
    {
        lua_newtable(L);
        int index = lua_absindex(L, -1);
        pushValueByName(L, index, key, v.key);
        pushValueByName(L, index, open, v.open);
        pushValueByName(L, index, high, v.high);
        pushValueByName(L, index, low, v.low);
        pushValueByName(L, index, close, v.close);
    }

    static QCPFinancialData get (lua_State* L, int index)
    {
        Q_ASSERT(lua_istable(L, index));

        QCPFinancialData d;
        d.key = getValueByName<double>(L, index, key);
        d.open = getValueByName<double>(L, index, open);
        d.high = getValueByName<double>(L, index, high);
        d.low = getValueByName<double>(L, index, low);
        d.close = getValueByName<double>(L, index, close);

        return d;
    }
};
const char* Stack<QCPFinancialData const&>::key = "key";
const char* Stack<QCPFinancialData const&>::open = "open";
const char* Stack<QCPFinancialData const&>::high = "high";
const char* Stack<QCPFinancialData const&>::low = "low";
const char* Stack<QCPFinancialData const&>::close = "close";

template <>
struct Stack<QCPFinancialData>
    : public Stack<QCPFinancialData const&>
{};

template <>
struct Stack<QCPCurveData const&>
    : public TablePushAndGet
{
    static const char* t;
    static const char* key;
    static const char* value;
    static void push (lua_State* L, QCPCurveData const& v)
    {
        lua_newtable(L);
        int index = lua_absindex(L, -1);
        pushValueByName(L, index, t, v.t);
        pushValueByName(L, index, key, v.key);
        pushValueByName(L, index, value, v.value);
    }

    static QCPCurveData get (lua_State* L, int index)
    {
        Q_ASSERT(lua_istable(L, index));

        QCPCurveData d;
        d.t = getValueByName<double>(L, index, t);
        d.key = getValueByName<double>(L, index, key);
        d.value = getValueByName<double>(L, index, value);

        return d;
    }

};
const char* Stack<QCPCurveData const&>::t = "t";
const char* Stack<QCPCurveData const&>::key = "key";
const char* Stack<QCPCurveData const&>::value = "value";

template <>
struct Stack<QCPCurveData>
    : public Stack<QCPCurveData const&>
{};

template <>
struct Stack<QCPGraphData const&>
    : public TablePushAndGet
{
    static const char* key;
    static const char* value;
    static void push (lua_State* L, QCPGraphData const& v)
    {
        lua_newtable(L);
        int index = lua_absindex(L, -1);
        pushValueByName(L, index, key, v.key);
        pushValueByName(L, index, value, v.value);
    }

    static QCPGraphData get (lua_State* L, int index)
    {
        Q_ASSERT(lua_istable(L, index));

        QCPGraphData d;
        d.key = getValueByName<double>(L, index, key);
        d.value = getValueByName<double>(L, index, value);

        return d;
    }
};
const char* Stack<QCPGraphData const&>::key = "key";
const char* Stack<QCPGraphData const&>::value = "value";

template <>
struct Stack<QCPGraphData>
    : public Stack<QCPGraphData const&>
{};

template <>
struct Stack<QCPStatisticalBoxData const&>
    : public TablePushAndGet
{
    static const char* key;
    static const char* minimum;
    static const char* lowerQuartile;
    static const char* median;
    static const char* upperQuartile;
    static const char* maximum;
    static const char* outliers;
    static void push (lua_State* L, QCPStatisticalBoxData const& v)
    {
        lua_newtable(L);
        int index = lua_absindex(L, -1);
        pushValueByName(L, index, key, v.key);
        pushValueByName(L, index, minimum, v.minimum);
        pushValueByName(L, index, lowerQuartile, v.lowerQuartile);
        pushValueByName(L, index, median, v.median);
        pushValueByName(L, index, upperQuartile, v.upperQuartile);
        pushValueByName(L, index,  maximum, v.maximum);
        pushValueByName(L, index, outliers, v.outliers);
    }

    static QCPStatisticalBoxData get (lua_State* L, int index)
    {
        Q_ASSERT(lua_istable(L, index));

        QCPStatisticalBoxData d;
        d.key = getValueByName<double>(L, index, key);
        d.minimum = getValueByName<double>(L, index, minimum);
        d.lowerQuartile = getValueByName<double>(L, index, lowerQuartile);
        d.median = getValueByName<double>(L, index, median);
        d.upperQuartile = getValueByName<double>(L, index, upperQuartile);
        d.maximum = getValueByName<double>(L, index, maximum);
        d.outliers = getValueByName<QVector<double>>(L, index, outliers);

        return d;
    }
};
const char* Stack<QCPStatisticalBoxData const&>::key = "key";
const char* Stack<QCPStatisticalBoxData const&>::minimum = "minimum";
const char* Stack<QCPStatisticalBoxData const&>::lowerQuartile = "lowerQuartile";
const char* Stack<QCPStatisticalBoxData const&>::median = "median";
const char* Stack<QCPStatisticalBoxData const&>::upperQuartile = "upperQuartile";
const char* Stack<QCPStatisticalBoxData const&>::maximum = "maximum";
const char* Stack<QCPStatisticalBoxData const&>::outliers = "outliers";

template <>
struct Stack<QCPStatisticalBoxData>
    : public Stack<QCPStatisticalBoxData const&>
{};

}   // namespace luabridge end.

#endif // TYPE4STACK_H
