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

STACK_AS_INT(QCP::AntialiasedElement)
STACK_AS_INT(QCP::MarginSide)
STACK_AS_INT(QCP::PlottingHint)
STACK_AS_INT(QCPAxis::AxisType)
STACK_AS_INT(QCPAxis::LabelSide)
STACK_AS_INT(QCPAxis::SelectablePart)
STACK_AS_INT(QCPAxis::ScaleType)
STACK_AS_INT(QCPAxisTicker::TickStepStrategy)
STACK_AS_INT(QCPAxisTickerFixed::ScaleStrategy)
STACK_AS_INT(QCPAxisTickerPi::FractionStyle)
STACK_AS_INT(QCPAxisTickerTime::TimeUnit)
STACK_AS_INT(QCPBars::WidthType)
STACK_AS_INT(QCPBarsGroup::SpacingType)
STACK_AS_INT(QCPColorGradient::ColorInterpolation)
STACK_AS_INT(QCPColorGradient::GradientPreset)
STACK_AS_INT(QCPCurve::LineStyle)
STACK_AS_INT(QCPErrorBars::ErrorType)
STACK_AS_INT(QCPFinancial::WidthType)
STACK_AS_INT(QCPFinancial::ChartStyle)
STACK_AS_INT(QCPGraph::LineStyle)
STACK_AS_INT(QCPItemBracket::BracketStyle)
STACK_AS_INT(QCPItemPosition::PositionType)
STACK_AS_INT(QCPItemTracer::TracerStyle)
STACK_AS_INT(QCPLayer::LayerMode)
STACK_AS_INT(QCPLayoutElement::SizeConstraintRect)
STACK_AS_INT(QCPLayoutElement::UpdatePhase)
STACK_AS_INT(QCPLayoutGrid::FillOrder)
STACK_AS_INT(QCPLayoutInset::InsetPlacement)
STACK_AS_INT(QCPLegend::SelectablePart)
STACK_AS_INT(QCPLineEnding::EndingStyle)
STACK_AS_INT(QCPPainter::PainterMode)
STACK_AS_INT(QCPScatterStyle::ScatterProperty)
STACK_AS_INT(QCPScatterStyle::ScatterShape)
STACK_AS_INT(QCPSelectionDecoratorBracket::BracketStyle)
STACK_AS_INT(QCustomPlot::LayerInsertMode)
STACK_AS_INT(QCustomPlot::RefreshPriority)

STACK_AS_INT_FROM(Qt::Alignment, Qt::AlignmentFlag)
STACK_AS_INT_FROM(Qt::ImageConversionFlags, Qt::ImageConversionFlag)

STACK_AS_INT_CONST_FROM(Qt::Alignment, Qt::AlignmentFlag)
STACK_AS_INT_CONST_FROM(Qt::ImageConversionFlags, Qt::ImageConversionFlag)

STACK_AS_INT_FROM(QCP::AntialiasedElements, QCP::AntialiasedElement)
STACK_AS_INT_FROM(QCP::Interactions, QCP::Interaction)
STACK_AS_INT_FROM(QCP::MarginSides, QCP::MarginSide)
STACK_AS_INT_FROM(QCP::PlottingHints, QCP::PlottingHint)
STACK_AS_INT_FROM(QCPAxis::AxisTypes, QCPAxis::AxisType)

STACK_AS_INT_CONST_FROM(QCP::AntialiasedElements, QCP::AntialiasedElement)
STACK_AS_INT_CONST_FROM(QCP::Interactions, QCP::Interaction)
STACK_AS_INT_CONST_FROM(QCP::MarginSides, QCP::MarginSide)
STACK_AS_INT_CONST_FROM(QCP::PlottingHints, QCP::PlottingHint)
STACK_AS_INT_CONST_FROM(QCPAxis::AxisTypes, QCPAxis::AxisType)

#undef STACK_AS_INT
#undef STACK_AS_INT_FROM
#undef STACK_AS_INT_CONST_FROM

// --

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

template <>
struct Stack <QString>
{
    static void push (lua_State* L, QString s)
    {
        Stack<QString const&>::push(L, s);
    }
    static QString get (lua_State* L, int index)
    {
        return Stack<QString const&>::get(L, index);
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
struct Stack <QVector<T> const&>
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

// --

template <>
struct Stack <QCPBarsData const&> : public TablePushAndGet
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
struct Stack <QCPBarsData>
{
    static void push (lua_State* L, QCPBarsData const& v)
    {
        Stack<QCPBarsData const&>::push(L, v);
    }

    static QCPBarsData get (lua_State* L, int index)
    {
        return Stack<QCPBarsData const&>::get(L, index);
    }
};

template <>
struct Stack <QCPErrorBarsData const&> : public TablePushAndGet
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
struct Stack <QCPErrorBarsData>
{
    static void push (lua_State* L, QCPErrorBarsData const& v)
    {
        Stack<QCPErrorBarsData const&>::push(L, v);
    }

    static QCPErrorBarsData get (lua_State* L, int index)
    {
        return Stack<QCPErrorBarsData const&>::get(L, index);
    }
};

template <>
struct Stack <QCPFinancialData const&> : public TablePushAndGet
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
struct Stack <QCPFinancialData>
{
    static void push (lua_State* L, QCPFinancialData const& v)
    {
        Stack<QCPFinancialData const&>::push(L, v);
    }

    static QCPFinancialData get (lua_State* L, int index)
    {
        return Stack<QCPFinancialData const&>::get(L, index);
    }
};

template <>
struct Stack <QCPCurveData const&> : public TablePushAndGet
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

template <>
struct Stack <QCPGraphData const&> : public TablePushAndGet
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
struct Stack <QCPStatisticalBoxData const&> : public TablePushAndGet
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
struct Stack <QCPStatisticalBoxData>
{
    static void push (lua_State* L, QCPStatisticalBoxData const& v)
    {
        Stack<QCPStatisticalBoxData const&>::push(L, v);
    }

    static QCPStatisticalBoxData get (lua_State* L, int index)
    {
        return Stack<QCPStatisticalBoxData const&>::get(L, index);
    }
};

}   // namespace luabridge end.

#endif // TYPE4STACK_H
