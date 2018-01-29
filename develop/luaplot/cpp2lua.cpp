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

// --

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

struct FontConstructor
{
    static QFont fromFont(const QFont& f) {
        return QFont(f);
    }

    static QFont fromFamily(const QString &family, int pointSize = -1, int weight = -1, bool italic = false) {
        return QFont(family, pointSize, weight, italic);
    }
};

struct LinearGradientConstructor
{
    static QLinearGradient fromXY(qreal xStart, qreal yStart, qreal xFinalStop, qreal yFinalStop) {
        return QLinearGradient(xStart, yStart, xFinalStop, yFinalStop);
    }
};

struct LocaleConstructor
{
    static QLocale fromString(const QString &name) {
        return QLocale(name);
    }

    static QLocale fromLanguageAndCountry(QLocale::Language language, QLocale::Country country = QLocale::AnyCountry) {
        return QLocale(language, country);
    }

    static QLocale fromLanguageScriptAndCountry(QLocale::Language language, QLocale::Script script, QLocale::Country country) {
        return QLocale(language, script, country);
    }

    static QLocale fromLocale(const QLocale &other) {
        return QLocale(other);
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

struct RadialGradientConstructor
{
    static QRadialGradient fromXYRadius(qreal cx, qreal cy, qreal radius) {
        return QRadialGradient(cx, cy, radius);
    }
};

struct ScatterStyleConstructor
{
    static QCPScatterStyle fromShapeAndSize(QCPScatterStyle::ScatterShape shape, double size=6) {
        return QCPScatterStyle(shape, size);
    }

    static QCPScatterStyle fromShapePenBrushAndSize(QCPScatterStyle::ScatterShape shape,
                                     const QPen &pen, const QBrush &brush,
                                     double size) {
        return QCPScatterStyle(shape, pen, brush, size);
    }

    static QCPScatterStyle fromPainterPath(const QPainterPath &customPath, const QPen &pen, const QBrush &brush=Qt::NoBrush, double size=6) {
        return QCPScatterStyle(customPath, pen, brush, size);
    }
};

// --

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

        .beginClass<QLocale>("QLocale")
          .addConstructor<void(*)()>()
        .endClass()
        .beginClass<LocaleConstructor>("LocaleConstructor")
          .addStaticFunction("fromString", &LocaleConstructor::fromString)
          .addStaticFunction("fromLanguageAndCountry", &LocaleConstructor::fromLanguageAndCountry)
          .addStaticFunction("fromLanguageScriptAndCountry", &LocaleConstructor::fromLanguageScriptAndCountry)
          .addStaticFunction("fromLocale", &LocaleConstructor::fromLocale)
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
          .addConstructor<void(*)()>()
          .addFunction("lighter", &QColor::lighter)
        .endClass()
        .beginClass<ColorConstructor>("ColorConstructor")
          .addStaticFunction("fromString", &ColorConstructor::fromString)
          .addStaticFunction("fromRGB", &ColorConstructor::fromRGB)
          .addStaticFunction("fromGlobal", &ColorConstructor::fromGlobal)
        .endClass()

        .beginClass<QPen>("QPen")
          .addConstructor<void(*)()>()
          .addFunction("setColor", &QPen::setColor)
          .addFunction("setStyle", &QPen::setStyle)
          .addFunction("setWidth", &QPen::setWidth)
          .addFunction("setWidthF", &QPen::setWidthF)
        .endClass()
        .beginClass<PenConstructor>("PenConstructor")
          .addStaticFunction("fromColor", &PenConstructor::fromColor)
          .addStaticFunction("fromStyle", &PenConstructor::fromStyle)
          .addStaticFunction("fromBrush", &PenConstructor::fromBrush)
        .endClass()

        .beginClass<QBrush>("QBrush")
          .addConstructor<void(*)()>()
          .addFunction("setStyle", &QBrush::setStyle)
        .endClass()
        .beginClass<BrushConstructor>("BrushConstructor")
          .addStaticFunction("fromStyle", &BrushConstructor::fromStyle)
          .addStaticFunction("fromColor", &BrushConstructor::fromColor)
          .addStaticFunction("fromGradient", &BrushConstructor::fromGradient)
        .endClass()

        .beginClass<QFont>("QFont")
          .addConstructor<void(*)()>()
          .addFunction("family", &QFont::family)
          .addFunction("pointSize", &QFont::pointSize)
          .addFunction("setStyleName", &QFont::setStyleName)
          .addFunction("setPointSize", &QFont::setPointSize)
          .addFunction("setPointSizeF", &QFont::setPointSizeF)
          .addFunction("styleName", &QFont::styleName)
        .endClass()
        .beginClass<FontConstructor>("FontConstructor")
          .addStaticFunction("fromFont", &FontConstructor::fromFont)
          .addStaticFunction("fromFamily", &FontConstructor::fromFamily)
        .endClass()

        .beginClass<QGradient>("QGradient")
            .addFunction("setColorAt", &QGradient::setColorAt)
        .endClass()

          .deriveClass<QRadialGradient, QGradient>("QRadialGradient")
          .endClass()
          .beginClass<RadialGradientConstructor>("RadialGradientConstructor")
            .addStaticFunction("fromXYRadius", &RadialGradientConstructor::fromXYRadius)
          .endClass()

          .deriveClass<QLinearGradient, QGradient>("QLinearGradient")
            .addConstructor<void(*)()>()
            .addFunction("setColorAt", &QLinearGradient::setColorAt)
            .addFunction("setFinalStopXY", static_cast<void(QLinearGradient::*)(qreal, qreal)>(&QLinearGradient::setFinalStop))
            .addFunction("setStartXY", static_cast<void(QLinearGradient::*)(qreal, qreal)>(&QLinearGradient::setStart))
          .endClass()
          .beginClass<LinearGradientConstructor>("LinearGradientConstructor")
            .addStaticFunction("fromXY", &LinearGradientConstructor::fromXY)
          .endClass()

        .beginClass<QPainterPath>("QPainterPath")
          .addConstructor<void(*)()>()
          .addFunction("cubicToXY", static_cast<void(QPainterPath::*)(qreal, qreal, qreal, qreal, qreal, qreal)>(&QPainterPath::cubicTo))
        .endClass()

      .endNamespace()
    ;
}

static void QtWidget2Lua(lua_State* L, const char* ns)
{
    luabridge::getGlobalNamespace(L)
      .beginNamespace(ns)

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
            .addData("xAxis2", &QCustomPlot::xAxis2)
            .addData("yAxis", &QCustomPlot::yAxis)
            .addData("yAxis2", &QCustomPlot::yAxis2)
            .addFunction("addGraph", &QCustomPlot::addGraph)
            .addFunction("addLayer", &QCustomPlot::addLayer)
            .addFunction("axisRect", &QCustomPlot::axisRect)
            .addFunction("graph", static_cast<QCPGraph*(QCustomPlot::*)(int) const>(&QCustomPlot::graph))
            .addFunction("lastGraph", static_cast<QCPGraph*(QCustomPlot::*)() const>(&QCustomPlot::graph))
            .addFunction("layerByName", static_cast<QCPLayer*(QCustomPlot::*)(const QString&)const>(&QCustomPlot::layer))
            .addFunction("layerByIndex", static_cast<QCPLayer*(QCustomPlot::*)(int)const>(&QCustomPlot::layer))
            .addFunction("plotLayout", &QCustomPlot::plotLayout)
            .addFunction("replot", &QCustomPlot::replot)
            .addFunction("rescaleAxes", &QCustomPlot::rescaleAxes)
            .addFunction("setAutoAddPlottableToLegend", &QCustomPlot::setAutoAddPlottableToLegend)
            .addFunction("setBackground", static_cast<void(QCustomPlot::*)(const QBrush&)>(&QCustomPlot::setBackground))
            .addFunction("setInteractions", &QCustomPlot::setInteractions)
            .addFunction("setLocale", &QCustomPlot::setLocale)
            .addFunction("setNoAntialiasingOnDrag", &QCustomPlot::setNoAntialiasingOnDrag)
          .endClass()

            .deriveClass<LuaPlot, QCustomPlot>("LuaPlot")
              .addConstructor<void (*) (QWidget*)>()
              .addCFunction("setLuaState", &LuaPlot::setLuaState)

              .addFunction("createAxisRect", &LuaPlot::createAxisRect)
              .addFunction("createColorMap", &LuaPlot::createColorMap)
              .addFunction("createColorScale", &LuaPlot::createColorScale)
              .addFunction("createAxisTickerFixed", &LuaPlot::createAxisTickerFixed)
              .addFunction("createAxisTickerDateTime", &LuaPlot::createAxisTickerDateTime)
              .addFunction("createAxisTickerLog", &LuaPlot::createAxisTickerLog)
              .addFunction("createAxisTickerText", &LuaPlot::createAxisTickerText)
              .addFunction("createBars", &LuaPlot::createBars)
              .addFunction("createErrorBars", &LuaPlot::createErrorBars)
              .addFunction("createLayoutGrid", &LuaPlot::createLayoutGrid)
              .addFunction("createMarginGroup", &LuaPlot::createMarginGroup)
              .addFunction("createGraphDataContainer", &LuaPlot::createDataContainer<QCPGraphDataContainer>)
              .addFunction("createFinancial", &LuaPlot::createFinancial)
              .addFunction("createCurve", &LuaPlot::createCurve)
              .addFunction("createItemTracer", &LuaPlot::createItemTracer)
              .addFunction("createItemCurve", &LuaPlot::createItemCurve)
              .addFunction("createItemText", &LuaPlot::createItemText)
              .addFunction("createItemBracket", &LuaPlot::createItemBracket)
              .addFunction("createStatisticalBox", &LuaPlot::createStatisticalBox)
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
          .addFunction("setVector", static_cast<void(QCPGraphDataContainer::*)(const QVector<QCPGraphData>&, bool)>(&QCPGraphDataContainer::set))
          .addFunction("setContainer", static_cast<void(QCPGraphDataContainer::*)(const QCPGraphDataContainer&)>(&QCPGraphDataContainer::set))
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
            .addFunction("addTick", &QCPAxisTickerText::addTick)
            .addFunction("addTicks", static_cast<void(QCPAxisTickerText::*)(const QVector<double>&, const QVector<QString>&)>(&QCPAxisTickerText::addTicks))
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
          .addStaticFunction("fromShapeAndSize", &ScatterStyleConstructor::fromShapeAndSize)
          .addStaticFunction("fromShapePenBrushAndSize", &ScatterStyleConstructor::fromShapePenBrushAndSize)
          .addStaticFunction("fromPainterPath", &ScatterStyleConstructor::fromPainterPath)
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
            .addFunction("moveRange", &QCPAxis::moveRange)
            .addFunction("range", &QCPAxis::range)
            .addFunction("scaleRange", static_cast<void(QCPAxis::*)(double, double)>(&QCPAxis::scaleRange))
            .addFunction("setBasePen", &QCPAxis::setBasePen)
            .addFunction("setPadding", &QCPAxis::setPadding)
            .addFunction("setRange", static_cast<void(QCPAxis::*)(double, double)>(&QCPAxis::setRange))
            .addFunction("setLabel", &QCPAxis::setLabel)
            .addFunction("setLabelColor", &QCPAxis::setLabelColor)
            .addFunction("setNumberFormat", &QCPAxis::setNumberFormat)
            .addFunction("setNumberPrecision", &QCPAxis::setNumberPrecision)
            .addFunction("setScaleType", &QCPAxis::setScaleType)
            .addFunction("setSubTickPen", &QCPAxis::setSubTickPen)
            .addFunction("setSubTicks", &QCPAxis::setSubTicks)
            .addFunction("setTicker", &QCPAxis::setTicker)
            .addFunction("setTickLabels", &QCPAxis::setTickLabels)
            .addFunction("setTickLabelColor", &QCPAxis::setTickLabelColor)
            .addFunction("setTickLabelFont", &QCPAxis::setTickLabelFont)
            .addFunction("setTickLabelRotation", &QCPAxis::setTickLabelRotation)
            .addFunction("setTickLength", &QCPAxis::setTickLength)
            .addFunction("setTickPen", &QCPAxis::setTickPen)
            .addFunction("setTicks", &QCPAxis::setTicks)
            .addFunction("setUpperEnding", &QCPAxis::setUpperEnding)
            .addFunction("ticker", &QCPAxis::ticker)
          .endClass()

          .deriveClass<QCPGrid, QCPLayerable>("Grid")
            .addFunction("setPen", &QCPGrid::setPen)
            .addFunction("setSubGridPen", &QCPGrid::setSubGridPen)
            .addFunction("setSubGridVisible", &QCPGrid::setSubGridVisible)
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
          .addFunction("keyAxis", &QCPAbstractPlottable::keyAxis)
          .addFunction("removeFromLegend", static_cast<bool(QCPAbstractPlottable::*)()const>(&QCPAbstractPlottable::removeFromLegend))
          .addFunction("removeLegendFromLegend", static_cast<bool(QCPAbstractPlottable::*)(QCPLegend*)const>(&QCPAbstractPlottable::removeFromLegend))
          .addFunction("rescaleAxes", &QCPAbstractPlottable::rescaleAxes)
          .addFunction("rescaleKeyAxis", &QCPAbstractPlottable::rescaleKeyAxis)
          .addFunction("setAntialiasedFill", &QCPAbstractPlottable::setAntialiasedFill)
          .addFunction("setBrush", &QCPAbstractPlottable::setBrush)
          .addFunction("setName", &QCPAbstractPlottable::setName)
          .addFunction("setPen", &QCPAbstractPlottable::setPen)
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
            .addFunction("setDataPlottable", &QCPErrorBars::setDataPlottable)
            .addFunction("setVector1", static_cast<void(QCPErrorBars::*)(const QVector<double>&)>(&QCPErrorBars::setData))
            .addFunction("setVector2", static_cast<void(QCPErrorBars::*)(const QVector<double>&, const QVector<double>&)>(&QCPErrorBars::setData))
          .endClass()

          .deriveClass<QCPAbstractPlottable1D<QCPBarsData>, QCPAbstractPlottable>("AbstractPlottable1D_BarsData")
          .endClass()
            .deriveClass<QCPBars, QCPAbstractPlottable1D<QCPBarsData> >("Bars")
              .addFunction("addData", static_cast<void(QCPBars::*)(double, double)>(&QCPBars::addData))
              .addFunction("moveAbove", &QCPBars::moveAbove)
              .addFunction("setVector", static_cast<void(QCPBars::*)(const QVector<double>&, const QVector<double>&, bool)>(&QCPBars::setData))
              .addFunction("setStackingGap", &QCPBars::setStackingGap)
              .addFunction("setWidth", &QCPBars::setWidth)
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
              .addFunction("setVector", static_cast<void(QCPGraph::*)(const QVector<double>&, const QVector<double>&, bool)>(&QCPGraph::setData))
              .addFunction("addVector", static_cast<void(QCPGraph::*)(const QVector<double>&, const QVector<double>&, bool)>(&QCPGraph::addData))
              .addFunction("setLineStyle", &QCPGraph::setLineStyle)
              .addFunction("setScatterStyle", &QCPGraph::setScatterStyle)
              .addFunction("setChannelFillGraph", &QCPGraph::setChannelFillGraph)
            .endClass()

          .deriveClass<QCPAbstractPlottable1D<QCPStatisticalBoxData>, QCPAbstractPlottable>("QCPAbstractPlottable1D_QCPStatisticalBoxData")
          .endClass()
            .deriveClass<QCPStatisticalBox, QCPAbstractPlottable1D<QCPStatisticalBoxData>>("StatisticalBox")
              .addFunction("addData", static_cast<void(QCPStatisticalBox::*)(double, double, double, double, double, double, const QVector<double>&)>(&QCPStatisticalBox::addData))
              .addFunction("addVector", static_cast<void(QCPStatisticalBox::*)(const QVector<double>&, const QVector<double>&, const QVector<double>&, const QVector<double>&, const QVector<double>&, const QVector<double>&, bool)>(&QCPStatisticalBox::addData))
              .addFunction("setBrush", &QCPStatisticalBox::setBrush)
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
            .addFunction("addAxes", &QCPAxisRect::addAxes)
            .addFunction("addAxis", &QCPAxisRect::addAxis)
            .addFunction("axis", &QCPAxisRect::axis)
            .addFunction("insetLayout", &QCPAxisRect::insetLayout)
            .addFunction("setBackgroundByBrush", static_cast<void(QCPAxisRect::*)(const QBrush&)>(&QCPAxisRect::setBackground))
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
                .addFunction("itemCount", &QCPLegend::itemCount)
                .addFunction("removeItemByIndex", static_cast<bool(QCPLegend::*)(int)>(&QCPLegend::removeItem))
                .addFunction("removeItem", static_cast<bool(QCPLegend::*)(QCPAbstractLegendItem*)>(&QCPLegend::removeItem))
                .addFunction("setBorderPen", &QCPLegend::setBorderPen)
                .addFunction("setBrush", &QCPLegend::setBrush)
                .addFunction("setFont", &QCPLegend::setFont)
              .endClass()

            .deriveClass<QCPLayoutInset, QCPLayout>("LayoutInset")
              .addFunction("setInsetAlignment", &QCPLayoutInset::setInsetAlignment)
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
