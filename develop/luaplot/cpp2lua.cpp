#include <lua.hpp>
#include <QDebug>
#include "LuaBridge/LuaBridge.h"
#include "qcp/qcustomplot.h"
#include "utilities.h"
#include "type4stack.h"
#include "luaplot.h"
#include "mainwindow.h"
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

// --

void connectProxy(const QObject *sender, const char *signal,
             const QObject *receiver, const char *member,
             Qt::ConnectionType t = Qt::AutoConnection)
{
//    qDebug() <<"signal: " << signal;
//    qDebug() <<"member: " << member;

    qApp->connect(sender, QString("2").append(signal).toUtf8().constData(),
                  receiver, QString("1").append(member).toUtf8().constData(),
                  t);
}

//LuaPlot* getPlot()
//{
//    return LuaPlot::ms_plot;
//}

struct RadialGradientConstructor
{
    static QRadialGradient fromXYRadius(qreal cx, qreal cy, qreal radius) {
        return QRadialGradient(cx, cy, radius);
    }
};

struct ScatterStyleConstructor
{
    static QCPScatterStyle fromShape(QCPScatterStyle::ScatterShape shape,
                                     const QPen &pen, const QBrush &brush,
                                     double size) {
        return QCPScatterStyle(shape, pen, brush, size);
    }
};

struct ColorConstructor
{
    static QColor fromString(const char* aname) {
        return QColor(aname);
    }

    static QColor fromRGB(int r, int g, int b, int a = 255) {
        return QColor(r, g, b, a);
    }

    static QColor fromGlobal(Qt::GlobalColor color) {
        return QColor(color);
    }
};

struct PenConstructor
{
    static QPen fromColor(const QColor& c) {
        return QPen(c);
    }

    static QPen fromStyle(Qt::PenStyle style) {
        return QPen(style);
    }

    static QPen fromBrush(const QBrush &brush, qreal width,
                          Qt::PenStyle style = Qt::SolidLine,
                          Qt::PenCapStyle cap = Qt::SquareCap,
                          Qt::PenJoinStyle join = Qt::BevelJoin) {
        return QPen(brush, width, style, cap, join);
    }
};

struct BrushConstructor
{
    static QBrush fromStyle(Qt::BrushStyle style) {
        return QBrush(style);
    }

    static QBrush fromColor(const QColor &color,
                            Qt::BrushStyle style = Qt::SolidPattern) {
        return QBrush(color, style);
    }

    static QBrush fromGradient(const QGradient &gradient) {
        return QBrush(gradient);
    }
};

static void QtCore2Lua(lua_State* L, const char* ns)
{
    luabridge::getGlobalNamespace(L)
      .beginNamespace(ns)

        .addFunction("connect", connectProxy)

        .beginClass<QDate>("QDate")
          .addConstructor<void(*)(int, int, int)>()
        .endClass()

        .beginClass<QDateTime>("QDateTime")
          .addConstructor<void(*)(const QDate&)>()
          .addFunction("setTimeSpec", &QDateTime::setTimeSpec)
          .addFunction("toTime_t", &QDateTime::toTime_t)
          .addStaticFunction("currentDateTime", &QDateTime::currentDateTime)
        .endClass()

        .beginClass<QMetaObject::Connection>("QMetaObject_Connection")
        .endClass()
        .beginClass<QObject>("QObject")
          .addStaticFunction("connect", static_cast<QMetaObject::Connection(*)(const QObject*, const char*, const QObject*, const char*, Qt::ConnectionType)>(QObject::connect))
        .endClass()

      .endNamespace()
    ;
}

static void QtGui2Lua(lua_State* L, const char* ns)
{
    luabridge::getGlobalNamespace(L)
      .beginNamespace(ns)

        .beginClass<QMargins>("QMargins")
          .addConstructor<void(*)(int, int, int, int)>()
        .endClass()

        .beginClass<QPointF>("QPointF")
          .addFunction("x", &QPointF::x)
          .addFunction("y", &QPointF::y)
        .endClass()

        .beginClass<QColor>("QColor")
        .endClass()
        .beginClass<ColorConstructor>("ColorConstructor")
          .addStaticFunction("fromString", &ColorConstructor::fromString)
          .addStaticFunction("fromRGB", &ColorConstructor::fromRGB)
          .addStaticFunction("fromGlobal", &ColorConstructor::fromGlobal)
        .endClass()

        .beginClass<QPen>("QPen")
          .addFunction("setWidthF", &QPen::setWidthF)
        .endClass()
        .beginClass<PenConstructor>("PenConstructor")
          .addStaticFunction("fromColor", &PenConstructor::fromColor)
          .addStaticFunction("fromStyle", &PenConstructor::fromStyle)
          .addStaticFunction("fromBrush", &PenConstructor::fromBrush)
        .endClass()

        .beginClass<QBrush>("QBrush")
        .endClass()
        .beginClass<BrushConstructor>("BrushConstructor")
          .addStaticFunction("fromStyle", &BrushConstructor::fromStyle)
          .addStaticFunction("fromColor", &BrushConstructor::fromColor)
          .addStaticFunction("fromGradient", &BrushConstructor::fromGradient)
        .endClass()

        .beginClass<QFont>("QFont")
          .addConstructor<void(*)(const QString &, int, int, bool)>()
          .addFunction("family", &QFont::family)
        .endClass()

        .beginClass<QGradient>("QGradient")
            .addFunction("setColorAt", &QGradient::setColorAt)
        .endClass()

          .deriveClass<QRadialGradient, QGradient>("QRadialGradient")
          .endClass()
          .beginClass<RadialGradientConstructor>("RadialGradientConstructor")
            .addStaticFunction("fromXYRadius", &RadialGradientConstructor::fromXYRadius)
          .endClass()

      .endNamespace()
    ;
}

static void QtWidget2Lua(lua_State* L, const char* ns)
{
    luabridge::getGlobalNamespace(L)
      .beginNamespace(ns)

//        .addFunction("getPlot", getPlot)

        .deriveClass<QApplication, QObject>("App")
          .addStaticFunction("exec", &QApplication::exec)
        .endClass()

        .deriveClass<QWidget, QObject>("Widget")
          .addFunction("setWindowTitle", &QWidget::setWindowTitle)
          .addFunction("show", &QWidget::show)
          .addFunction("setAttribute", &QWidget::setAttribute)
          .addFunction("resize", static_cast<void (QWidget::*)(int, int)>(&QWidget::resize))
          .addFunction("parentWidget", &QWidget::parentWidget)
          .addFunction("font", &QWidget::font)
        .endClass()

          .deriveClass<QCustomPlot, QWidget>("CustomPlot")
            .addData("legend", &QCustomPlot::legend)
            .addData("xAxis", &QCustomPlot::xAxis)
            .addData("yAxis", &QCustomPlot::yAxis)
            .addData("xAxis2", &QCustomPlot::xAxis2)
            .addData("yAxis2", &QCustomPlot::yAxis2)
            .addFunction("plotLayout", &QCustomPlot::plotLayout)
            .addFunction("addGraph", &QCustomPlot::addGraph)
            .addFunction("graph", static_cast<QCPGraph*(QCustomPlot::*)(int) const>(&QCustomPlot::graph))
            .addFunction("setAutoAddPlottableToLegend", &QCustomPlot::setAutoAddPlottableToLegend)
            .addFunction("rescaleAxes", &QCustomPlot::rescaleAxes)
            .addFunction("axisRect", &QCustomPlot::axisRect)
            .addFunction("setInteractions", &QCustomPlot::setInteractions)
            .addFunction("replot", &QCustomPlot::replot)
          .endClass()

            .deriveClass<LuaPlot, QCustomPlot>("LuaPlot")
              .addConstructor<void (*) (QWidget*)>()
              .addCFunction("setLuaState", &LuaPlot::setLuaState)

              .addFunction("createAxisRect", &LuaPlot::createAxisRect)
              .addFunction("createColorMap", &LuaPlot::createColorMap)
              .addFunction("createColorScale", &LuaPlot::createColorScale)
              .addFunction("createAxisTickerFixed", &LuaPlot::createAxisTickerFixed)
              .addFunction("createAxisTkckerDateTime", &LuaPlot::createAxisTickerDateTime)
              .addFunction("createBars", &LuaPlot::createBars)
              .addFunction("createLayoutGrid", &LuaPlot::createLayoutGrid)
              .addFunction("createMarginGroup", &LuaPlot::createMarginGroup)
              .addFunction("createGraphDataContainer", &LuaPlot::createDataContainer<QCPGraphDataContainer>)
              .addFunction("createFinancial", &LuaPlot::createFinancial)
              .addFunction("createCurve", &LuaPlot::createCurve)
              .addFunction("createItemTracer", &LuaPlot::createItemTracer)
              .addFunction("createItemCurve", &LuaPlot::createItemCurve)
              .addFunction("createItemText", &LuaPlot::createItemText)
              .addFunction("createItemBracket", &LuaPlot::createItemBracket)
              .addFunction("setTimer", &LuaPlot::setTimer)
            .endClass()

          .deriveClass<QMainWindow, QWidget>("QMainWindow")
          .endClass()

            .deriveClass<MainWindow, QMainWindow>("MainWindow")
              .addConstructor<void (*) (QWidget* parent)>()
              .addFunction("getPlot", &MainWindow::getPlot)
            .endClass()

      .endNamespace()
    ;
}



static int cellToCoord(lua_State* L)
{
    // from lua: key, value = celltoCoord(colorMapData, keyIndex, valueIndex)

    int valueIndex = luabridge::Stack<int>::get(L, 3);
    int keyIndex = luabridge::Stack<int>::get(L, 2);
    QCPColorMapData* colorMapData = luabridge::Stack<QCPColorMapData*>::get(L, 1);
    lua_settop(L, 0);

    double key = 0.0, value = 0.0;
    colorMapData->cellToCoord(keyIndex, valueIndex, &key, &value);

    luabridge::Stack<double>::push(L, key);
    luabridge::Stack<double>::push(L, value);

    return 2;
}

static void QcpContainer2Lua(lua_State* L, const char* ns)
{
    luabridge::getGlobalNamespace(L)
      .beginNamespace(ns)

        .beginClass<QCPColorMapData>("ColorMapData")
          .addFunction("setCell", &QCPColorMapData::setCell)
          .addFunction("setRange", &QCPColorMapData::setRange)
          .addFunction("setSize", &QCPColorMapData::setSize)
//            .addFunction("cellToCoord", &QCPColorMapData::cellToCoord)
        .endClass()
        .beginNamespace("ColorMapDataHelper")
          .addCFunction("cellToCoord", cellToCoord)
        .endNamespace()

        .beginClass<QCPCurveDataContainer>("CurveDataContainer")
          .addFunction("addContainer", static_cast<void(QCPCurveDataContainer::*)(const QCPCurveDataContainer&)>(&QCPCurveDataContainer::add))
          .addFunction("addVector", static_cast<void(QCPCurveDataContainer::*)(const QVector<QCPCurveData>&, bool)>(&QCPCurveDataContainer::add))
          .addFunction("addData", static_cast<void(QCPCurveDataContainer::*)(const QCPCurveData&)>(&QCPCurveDataContainer::add))
          .addFunction("size", &QCPCurveDataContainer::size)
        .endClass()

        .beginClass<QCPFinancialDataContainer>("FinancialDataContainer")
          .addFunction("addContainer", static_cast<void(QCPFinancialDataContainer::*)(const QCPFinancialDataContainer&)>(&QCPFinancialDataContainer::add))
          .addFunction("addVector", static_cast<void(QCPFinancialDataContainer::*)(const QVector<QCPFinancialData>&, bool)>(&QCPFinancialDataContainer::add))
          .addFunction("addData", static_cast<void(QCPFinancialDataContainer::*)(const QCPFinancialData&)>(&QCPFinancialDataContainer::add))
          .addFunction("size", &QCPFinancialDataContainer::size)
        .endClass()

        .beginClass<QCPGraphDataContainer>("GraphDataContainer")
          .addFunction("addContainer", static_cast<void(QCPGraphDataContainer::*)(const QCPGraphDataContainer&)>(&QCPGraphDataContainer::add))
          .addFunction("addVector", static_cast<void(QCPGraphDataContainer::*)(const QVector<QCPGraphData>&, bool)>(&QCPGraphDataContainer::add))
          .addFunction("addData", static_cast<void(QCPGraphDataContainer::*)(const QCPGraphData&)>(&QCPGraphDataContainer::add))
          .addFunction("size", &QCPGraphDataContainer::size)
        .endClass()

      .endNamespace()
    ;
}

static void QcpBasic2Lua(lua_State* L, const char* ns)
{
    luabridge::getGlobalNamespace(L)
      .beginNamespace(ns)

        .beginClass<QCPAxisTicker>("AxisTicker")
          .addFunction("setTickCount", &QCPAxisTicker::setTickCount)
        .endClass()   

          .deriveClass<QCPAxisTickerDateTime, QCPAxisTicker>("AxisTickerDateTime")
            .addFunction("setDateTimeSpec", &QCPAxisTickerDateTime::setDateTimeSpec)
            .addFunction("setDateTimeFormat", &QCPAxisTickerDateTime::setDateTimeFormat)
            .addStaticFunction("dateTimeToKey", static_cast<double(*)(const QDateTime)>(&QCPAxisTickerDateTime::dateTimeToKey))
          .endClass()

          .deriveClass<QCPAxisTickerFixed, QCPAxisTicker>("AxisTickerFixed")
            .addFunction("setTickStep", &QCPAxisTickerFixed::setTickStep)
            .addFunction("setScaleStrategy", &QCPAxisTickerFixed::setScaleStrategy)
          .endClass()

          .deriveClass<QCPAxisTickerLog, QCPAxisTicker>("AxisTickerLog")
          .endClass()

          .deriveClass<QCPAxisTickerPi, QCPAxisTicker>("AxisTickerPi")
          .endClass()

          .deriveClass<QCPAxisTickerText, QCPAxisTicker>("AxisTickerText")
          .endClass()

          .deriveClass<QCPAxisTickerTime, QCPAxisTicker>("AxisTickerTime")
          .endClass()

        .deriveClass<QCPBarsGroup, QObject>("BarsGroup")
        .endClass()

        .beginClass<QCPColorGradient>("ColorGradient")
          .addConstructor<void(*)(QCPColorGradient::GradientPreset)>()
        .endClass()

        .beginClass<QCPDataSelection>("DataSelection")
        .endClass()

        .beginClass<QCPItemAnchor>("ItemAnchor")
          .addFunction("pixelPosition", &QCPItemAnchor::pixelPosition)
        .endClass()

          .deriveClass<QCPItemPosition, QCPItemAnchor>("ItemPosition")
            .addFunction("setParentAnchor", &QCPItemPosition::setParentAnchor)
            .addFunction("setCoords", static_cast<void(QCPItemPosition::*)(double, double)>(&QCPItemPosition::setCoords))
            .addFunction("setType", &QCPItemPosition::setType)
            .addFunction("key", &QCPItemPosition::key)
            .addFunction("value", &QCPItemPosition::value)
          .endClass()

        .deriveClass<QCPLayer, QObject>("Layer")
        .endClass()

        .beginClass<QCPLineEnding>("LineEnding")
          .addConstructor<void(*)(QCPLineEnding::EndingStyle, double, double, bool)>()
        .endClass()

        .deriveClass<QCPMarginGroup, QObject>("MarginGroup")
        .endClass()

        .beginClass<QCPRange>("Range")
          .addConstructor<void(*)(double, double)>()
          .addFunction("center", &QCPRange::center)
        .endClass()

        .beginClass<QCPScatterStyle>("ScatterStyle")
        .endClass()

        .beginClass<ScatterStyleConstructor>("ScatterStyleConstructor")
          .addStaticFunction("fromShape", &ScatterStyleConstructor::fromShape)
        .endClass()

        .beginClass<QCPSelectionDecorator>("QCPSelectionDecorator")
        .endClass()

          .deriveClass<QCPSelectionDecoratorBracket, QCPSelectionDecorator>("QCPSelectionDecoratorBracket")
          .endClass()

//        .beginClass<QCPStatisticalBoxData>("StatisticalBoxData")
//        .endClass()

        .beginClass<QCPVector2D>("QCPVector2D")
        .endClass()

      .endNamespace()
    ;
}

static void QcpLayerable2Lua(lua_State* L, const char* ns)
{
    luabridge::getGlobalNamespace(L)
      .beginNamespace(ns)

        .deriveClass<QCPLayerable, QObject>("Layerable")
          .addFunction("setAntialiased", &QCPLayerable::setAntialiased)
          .addFunction("setVisible", &QCPLayerable::setVisible)
          .addFunction("setLayer", static_cast<bool(QCPLayerable::*)(const QString&)>(&QCPLayerable::setLayer))
        .endClass()

          .deriveClass<QCPAxis, QCPLayerable>("Axis")
            .addFunction("grid", &QCPAxis::grid)
            .addFunction("range", &QCPAxis::range)
            .addFunction("scaleRange", static_cast<void(QCPAxis::*)(double, double)>(&QCPAxis::scaleRange))
            .addFunction("setBasePen", &QCPAxis::setBasePen)
            .addFunction("setRange", static_cast<void(QCPAxis::*)(double, double)>(&QCPAxis::setRange))
            .addFunction("setLabel", &QCPAxis::setLabel)
            .addFunction("setSubTicks", &QCPAxis::setSubTicks)
            .addFunction("setTicker", &QCPAxis::setTicker)
            .addFunction("setTickLabels", &QCPAxis::setTickLabels)
            .addFunction("setTickLabelColor", &QCPAxis::setTickLabelColor)
            .addFunction("setTickLabelRotation", &QCPAxis::setTickLabelRotation)
            .addFunction("setTicks", &QCPAxis::setTicks)
            .addFunction("ticker", &QCPAxis::ticker)
          .endClass()

          .deriveClass<QCPGrid, QCPLayerable>("Grid")
            .addFunction("setVisible", &QCPGrid::setVisible)
            .addFunction("setZeroLinePen", &QCPGrid::setZeroLinePen)
          .endClass()

          .deriveClass<QCPSelectionRect, QCPLayerable>("SelectionRect")
          .endClass()

      .endNamespace()
    ;
}

static void QcpItem2Lua(lua_State* L, const char* ns)
{
    luabridge::getGlobalNamespace(L)
      .beginNamespace(ns)

        .deriveClass<QCPAbstractItem, QCPLayerable>("AbstractItem")
        .endClass()

          .deriveClass<QCPItemBracket, QCPAbstractItem>("ItemBracket")
            .addData("left", &QCPItemBracket::left)
            .addData("right", &QCPItemBracket::right)
            .addData("center", &QCPItemBracket::center)
            .addFunction("setLength", &QCPItemBracket::setLength)
          .endClass()

          .deriveClass<QCPItemCurve, QCPAbstractItem>("ItemCurve")
            .addData("start", &QCPItemCurve::start)
            .addData("startDir", &QCPItemCurve::startDir)
            .addData("theEnd", &QCPItemCurve::end)
            .addData("endDir", &QCPItemCurve::endDir)
            .addFunction("setHead", &QCPItemCurve::setHead)
            .addFunction("setTail", &QCPItemCurve::setTail)
          .endClass()

          .deriveClass<QCPItemEllipse, QCPAbstractItem>("ItemEllipse")
          .endClass()

          .deriveClass<QCPItemLine, QCPAbstractItem>("ItemLine")
          .endClass()

          .deriveClass<QCPItemPixmap, QCPAbstractItem>("ItemPixmap")
          .endClass()

          .deriveClass<QCPItemRect, QCPAbstractItem>("ItemRect")
          .endClass()

          .deriveClass<QCPItemStraightLine, QCPAbstractItem>("ItemStraightLine")
          .endClass()

          .deriveClass<QCPItemText, QCPAbstractItem>("ItemText")
            .addData("position", &QCPItemText::position)
            .addData("left", &QCPItemText::left)
            .addData("top", &QCPItemText::top)
            .addData("bottom", &QCPItemText::bottom)
            .addFunction("setPositionAlignment", &QCPItemText::setPositionAlignment)
            .addFunction("setText", &QCPItemText::setText)
            .addFunction("setTextAlignment", &QCPItemText::setTextAlignment)
            .addFunction("setFont", &QCPItemText::setFont)
            .addFunction("setPadding", &QCPItemText::setPadding)
            .addFunction("setRotation", &QCPItemText::setRotation)
          .endClass()

          .deriveClass<QCPItemTracer, QCPAbstractItem>("QCPItemTracer")
            .addData("position", &QCPItemTracer::position)
            .addFunction("setGraph", &QCPItemTracer::setGraph)
            .addFunction("setGraphKey", &QCPItemTracer::setGraphKey)
            .addFunction("setInterpolating", &QCPItemTracer::setInterpolating)
            .addFunction("setStyle", &QCPItemTracer::setStyle)
            .addFunction("setPen", &QCPItemTracer::setPen)
            .addFunction("setBrush", &QCPItemTracer::setBrush)
            .addFunction("setSize", &QCPItemTracer::setSize)
          .endClass()

      .endNamespace()
    ;
}

static void QcpPlottable2Lua(lua_State* L, const char* ns)
{
    luabridge::getGlobalNamespace(L)
      .beginNamespace(ns)

        .deriveClass<QCPAbstractPlottable, QCPLayerable>("AbstractPlottable")
          .addFunction("setName", &QCPAbstractPlottable::setName)
          .addFunction("setPen", &QCPAbstractPlottable::setPen)
          .addFunction("setBrush", &QCPAbstractPlottable::setBrush)
          .addFunction("keyAxis", &QCPAbstractPlottable::keyAxis)
          .addFunction("rescaleAxes", &QCPAbstractPlottable::rescaleAxes)
          .addFunction("rescaleKeyAxis", &QCPAbstractPlottable::rescaleKeyAxis)
          .addFunction("setAntialiasedFill", &QCPAbstractPlottable::setAntialiasedFill)
          .addFunction("valueAxis", &QCPAbstractPlottable::valueAxis)
        .endClass()

          .deriveClass<QCPColorMap, QCPAbstractPlottable>("ColorMap")
            .addFunction("data", &QCPColorMap::data)
            .addFunction("rescaleAxes", &QCPColorMap::rescaleAxes)
            .addFunction("rescaleDataRange", &QCPColorMap::rescaleDataRange)
            .addFunction("setGradient", &QCPColorMap::setGradient)
            .addFunction("setColorScale", &QCPColorMap::setColorScale)
          .endClass()

          .deriveClass<QCPErrorBars, QCPAbstractPlottable>("QCPErrorBars")
          .endClass()

          .deriveClass<QCPAbstractPlottable1D<QCPBarsData>, QCPAbstractPlottable>("AbstractPlottable1D_BarsData")
          .endClass()
            .deriveClass<QCPBars, QCPAbstractPlottable1D<QCPBarsData> >("Bars")
              .addFunction("setWidth", &QCPBars::setWidth)
              .addFunction("setData", static_cast<void(QCPBars::*)(const QVector<double>&, const QVector<double>&, bool)>(&QCPBars::setData))
              .addFunction("addData", static_cast<void(QCPBars::*)(double, double)>(&QCPBars::addData))
            .endClass()

          .deriveClass<QCPAbstractPlottable1D<QCPCurveData>, QCPAbstractPlottable>("QCPAbstractPlottable1D_QCPCurveData")
          .endClass()
            .deriveClass<QCPCurve, QCPAbstractPlottable1D<QCPCurveData>>("Curve")
              .addFunction("data", &QCPCurve::data)
            .endClass()

          .deriveClass<QCPAbstractPlottable1D<QCPFinancialData>, QCPAbstractPlottable>("QCPAbstractPlottable1D_QCPFinancialData")
          .endClass()
            .deriveClass<QCPFinancial, QCPAbstractPlottable1D<QCPFinancialData>>("Financial")
              .addFunction("data", &QCPFinancial::data)
              .addFunction("setChartStyle", &QCPFinancial::setChartStyle)
              .addFunction("setWidth", &QCPFinancial::setWidth)
              .addFunction("setTwoColored", &QCPFinancial::setTwoColored)
              .addFunction("setBrushPositive", &QCPFinancial::setBrushPositive)
              .addFunction("setBrushNegative", &QCPFinancial::setBrushNegative)
              .addFunction("setPenPositive", &QCPFinancial::setPenPositive)
              .addFunction("setPenNegative", &QCPFinancial::setPenNegative)
              .addStaticFunction("timeSeriesToOhlc", &QCPFinancial::timeSeriesToOhlc)
            .endClass()

          .deriveClass<QCPAbstractPlottable1D<QCPGraphData>, QCPAbstractPlottable>("QCPAbstractPlottable1D_QCPGraphData")
          .endClass()
            .deriveClass<QCPGraph, QCPAbstractPlottable1D<QCPGraphData> >("Graph")
              .addFunction("data", &QCPGraph::data)
              .addFunction("setData", static_cast<void(QCPGraph::*)(const QVector<double>&, const QVector<double>&, bool)>(&QCPGraph::setData))
              .addFunction("addData", static_cast<void(QCPGraph::*)(const QVector<double>&, const QVector<double>&, bool)>(&QCPGraph::addData))
              .addFunction("setLineStyle", &QCPGraph::setLineStyle)
              .addFunction("setScatterStyle", &QCPGraph::setScatterStyle)
              .addFunction("setChannelFillGraph", &QCPGraph::setChannelFillGraph)
            .endClass()

          .deriveClass<QCPAbstractPlottable1D<QCPStatisticalBoxData>, QCPAbstractPlottable>("QCPAbstractPlottable1D_QCPStatisticalBoxData")
          .endClass()
            .deriveClass<QCPStatisticalBox, QCPAbstractPlottable1D<QCPStatisticalBoxData>>("StatisticalBox")
            .endClass()

      .endNamespace()
    ;
}

static void QcpElement2Lua(lua_State* L, const char* ns)
{
    luabridge::getGlobalNamespace(L)
      .beginNamespace(ns)

        .deriveClass<QCPLayoutElement, QCPLayerable>("LayoutElement")
          .addFunction("setMarginGroup", &QCPLayoutElement::setMarginGroup)
          .addFunction("setAutoMargins", &QCPLayoutElement::setAutoMargins)
          .addFunction("setMargins", &QCPLayoutElement::setMargins)
        .endClass()

          .deriveClass<QCPAbstractLegendItem, QCPLayoutElement>("AbstractLegendItem")
          .endClass()

          .deriveClass<QCPPlottableLegendItem, QCPAbstractLegendItem>("QCPPlottableLegendItem")
          .endClass()

          .deriveClass<QCPAxisRect, QCPLayoutElement>("AxisRect")
            .addFunction("axis", &QCPAxisRect::axis)
            .addFunction("addAxes", &QCPAxisRect::addAxes)
            .addFunction("addAxis", &QCPAxisRect::addAxis)
            .addFunction("setMaximumSize", static_cast<void (QCPAxisRect::*)(int, int)>(&QCPAxisRect::setMaximumSize))
            .addFunction("setMinimumSize", static_cast<void (QCPAxisRect::*)(int, int)>(&QCPAxisRect::setMinimumSize))
            .addFunction("setupFullAxesBox", &QCPAxisRect::setupFullAxesBox)
          .endClass()

          .deriveClass<QCPColorScale, QCPLayoutElement>("ColorScale")
            .addFunction("axis", &QCPColorScale::axis)
            .addFunction("setType", &QCPColorScale::setType)
          .endClass()

          .deriveClass<QCPLayout, QCPLayoutElement>("Layout")
            .addFunction("clear", &QCPLayoutGrid::clear)
          .endClass()

            .deriveClass<QCPLayoutGrid, QCPLayout>("LayoutGrid")
              .addFunction("setColumnStretchFactor", &QCPLayoutGrid::setColumnStretchFactor)
              .addFunction("insertRow", &QCPLayoutGrid::insertRow)
              .addFunction("addElement", static_cast<bool (QCPLayoutGrid::*)(int, int, QCPLayoutElement*)>(&QCPLayoutGrid::addElement))
              .addFunction("setRowSpacing", &QCPLayoutGrid::setRowSpacing)
              .addFunction("elementCount", &QCPLayoutGrid::elementCount)
            .endClass()

              .deriveClass<QCPLegend, QCPLayoutGrid>("Legend")
              .endClass()

            .deriveClass<QCPLayoutInset, QCPLayout>("LayoutInset")
            .endClass()

          .deriveClass<QCPTextElement, QCPLayoutElement>("TextElement")
            .addFunction("setText", &QCPTextElement::setText)
          .endClass()

      .endNamespace()
    ;
}

static void Tester2Lua(lua_State* L, const char* ns)
{
    luabridge::getGlobalNamespace(L)
      .beginNamespace(ns)

        .addFunction("getQVector", getQVector)
        .addFunction("getQString", getQString)

//        .beginClass<aaa>("aaa")
//        .endClass()

//          .deriveClass<bbb, aaa>("bbb")
//          .endClass()

      .endNamespace()
    ;
}

void Cpp2Lua(lua_State* L, const char* ns)
{
    QtCore2Lua(L, ns);
    QtGui2Lua(L, ns);
    QtWidget2Lua(L, ns);
    QcpBasic2Lua(L, ns);
    QcpContainer2Lua(L, ns);
    QcpLayerable2Lua(L, ns);
    QcpItem2Lua(L, ns);
    QcpPlottable2Lua(L, ns);
    QcpElement2Lua(L, ns);

    Tester2Lua(L, ns);

//    lua_settop(L, 0);
//    lua_getglobal(L, ns);
}
