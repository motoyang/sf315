#include <lua.hpp>
#include <QDebug>
#include "LuaBridge/LuaBridge.h"
#include "qcp/qcustomplot.h"
#include "utilities.h"
#include "luaplot.h"
#include "mainwindow.h"
#include "type4stack.h"
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

    static QBrush fromPixmap(const QPixmap &pixmap) {
        return QBrush(pixmap);
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

struct PixmapConstructor
{
    static QPixmap fromFile(const QString &fileName, const char *format = Q_NULLPTR, Qt::ImageConversionFlags flags = Qt::AutoColor) {
        return QPixmap(fileName, format, flags);
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

    static QCPScatterStyle fromPixmap(const QPixmap &pixmap) {
        return QCPScatterStyle(pixmap);
    }
};

// --

static void QtCore2Lua(lua_State* L, const char* ns)
{
    luabridge::getGlobalNamespace(L)
      .beginNamespace(ns)

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

        .beginClass<QTime>("QTime")
          .addConstructor<void(*)()>()
          .addStaticFunction("currentTime", &QTime::currentTime)
          .addFunction("elapsed", &QTime::elapsed)
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
          .addConstructor<void(*)(double, double)>()
          .addFunction("x", &QPointF::x)
          .addFunction("y", &QPointF::y)
        .endClass()

        .beginClass<QRect>("QRect")
          .addConstructor<void(*)(int, int, int, int)>()
          .addFunction("width", &QRect::width)
          .addFunction("height", &QRect::height)
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
          .addStaticFunction("fromPixmap", &BrushConstructor::fromPixmap)
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

        .beginClass<QPixmap>("QPixmap")
          .addConstructor<void(*)()>()
          .addFunction("scaledXY", static_cast<QPixmap(QPixmap::*)(int, int, Qt::AspectRatioMode, Qt::TransformationMode)const>(&QPixmap::scaled))
        .endClass()
        .beginClass<PixmapConstructor>("PixmapConstructor")
          .addStaticFunction("fromFile", &PixmapConstructor::fromFile)
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
          .addFunction("hide", &QWidget::hide)
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
            .addFunction("antialiasedElements", &QCustomPlot::antialiasedElements)
            .addFunction("autoAddPlottableToLegend", &QCustomPlot::autoAddPlottableToLegend)
            .addFunction("axisRect", &QCustomPlot::axisRect)
            .addFunction("axisRectCount", &QCustomPlot::axisRectCount)
            .addFunction("axisRects", &QCustomPlot::axisRects)
            .addFunction("background", &QCustomPlot::background)
            .addFunction("backgroundScaled", &QCustomPlot::backgroundScaled)
            .addFunction("backgroundScaledMode", &QCustomPlot::backgroundScaledMode)
            .addFunction("bufferDevicePixelRatio", &QCustomPlot::bufferDevicePixelRatio)
            .addFunction("clearGraphs", &QCustomPlot::clearGraphs)
            .addFunction("clearItems", &QCustomPlot::clearItems)
            .addFunction("clearPlottables", &QCustomPlot::clearPlottables)
            .addFunction("currentLayer", &QCustomPlot::currentLayer)
            .addFunction("graph", static_cast<QCPGraph*(QCustomPlot::*)(int) const>(&QCustomPlot::graph))
            .addFunction("graphCount", &QCustomPlot::graphCount)
            .addFunction("hasPlottable", &QCustomPlot::hasPlottable)
            .addFunction("hasItem", &QCustomPlot::hasItem)
            .addFunction("interactions", &QCustomPlot::interactions)
            .addFunction("item", static_cast<QCPAbstractItem*(QCustomPlot::*)(int)const>(&QCustomPlot::item))
            .addFunction("itemCount", &QCustomPlot::itemCount)
            .addFunction("lastGraph", static_cast<QCPGraph*(QCustomPlot::*)() const>(&QCustomPlot::graph))
            .addFunction("lastItem", static_cast<QCPAbstractItem*(QCustomPlot::*)()const>(&QCustomPlot::item))
            .addFunction("lastPlottable", static_cast<QCPAbstractPlottable*(QCustomPlot::*)()>(&QCustomPlot::plottable))
            .addFunction("layerByName", static_cast<QCPLayer*(QCustomPlot::*)(const QString&)const>(&QCustomPlot::layer))
            .addFunction("layerByIndex", static_cast<QCPLayer*(QCustomPlot::*)(int)const>(&QCustomPlot::layer))
            .addFunction("layerCount", &QCustomPlot::layerCount)
            .addFunction("multiSelectModifier", &QCustomPlot::multiSelectModifier)
            .addFunction("moveLayer", &QCustomPlot::moveLayer)
            .addFunction("noAntialiasingOnDrag", &QCustomPlot::noAntialiasingOnDrag)
            .addFunction("notAntialiasedElements", &QCustomPlot::notAntialiasedElements)
            .addFunction("openGl", &QCustomPlot::openGl)
            .addFunction("plottable", static_cast<QCPAbstractPlottable*(QCustomPlot::*)(int)>(&QCustomPlot::plottable))
            .addFunction("plottableCount", &QCustomPlot::plottableCount)
            .addFunction("plotLayout", &QCustomPlot::plotLayout)
            .addFunction("plottingHints", &QCustomPlot::plottingHints)
            .addFunction("removeGraph", static_cast<bool(QCustomPlot::*)(QCPGraph*)>(&QCustomPlot::removeGraph))
            .addFunction("removeGraphByIndex", static_cast<bool(QCustomPlot::*)(int)>(&QCustomPlot::removeGraph))
            .addFunction("removeItem", static_cast<bool(QCustomPlot::*)(QCPAbstractItem*)>(&QCustomPlot::removeItem))
            .addFunction("removeItemByIndex", static_cast<bool(QCustomPlot::*)(int)>(&QCustomPlot::removeItem))
            .addFunction("removeLayer", &QCustomPlot::removeLayer)
            .addFunction("removePlottable", static_cast<bool(QCustomPlot::*)(QCPAbstractPlottable*)>(&QCustomPlot::removePlottable))
            .addFunction("removePlottableByIndex", static_cast<bool(QCustomPlot::*)(int)>(&QCustomPlot::removePlottable))
            .addFunction("replot", &QCustomPlot::replot)
            .addFunction("rescaleAxes", &QCustomPlot::rescaleAxes)
            .addFunction("saveBmp", &QCustomPlot::saveBmp)
            .addFunction("saveJpg", &QCustomPlot::saveJpg)
            .addFunction("savePdf", &QCustomPlot::savePdf)
            .addFunction("savePng", &QCustomPlot::savePng)
            .addFunction("setAntialiasedElement", &QCustomPlot::setAntialiasedElement)
            .addFunction("setAntialiasedElements", &QCustomPlot::setAntialiasedElements)
            .addFunction("setAutoAddPlottableToLegend", &QCustomPlot::setAutoAddPlottableToLegend)
            .addFunction("setBackgroundByPixmap", static_cast<void(QCustomPlot::*)(const QPixmap&, bool, Qt::AspectRatioMode)>(&QCustomPlot::setBackground))
            .addFunction("setBackgroundByBrush", static_cast<void(QCustomPlot::*)(const QBrush&)>(&QCustomPlot::setBackground))
            .addFunction("setBackgroundScaled", &QCustomPlot::setBackgroundScaled)
            .addFunction("setBufferDevicePixelRatio", &QCustomPlot::setBufferDevicePixelRatio)
            .addFunction("setCurrentLayer", static_cast<bool(QCustomPlot::*)(QCPLayer*)>(&QCustomPlot::setCurrentLayer))
            .addFunction("setCurrentLayerByName", static_cast<bool(QCustomPlot::*)(const QString&)>(&QCustomPlot::setCurrentLayer))
            .addFunction("setInteractions", &QCustomPlot::setInteractions)
            .addFunction("setInteraction", &QCustomPlot::setInteraction)
            .addFunction("setLocale", &QCustomPlot::setLocale)
            .addFunction("setNotAntialiasedElement", &QCustomPlot::setNotAntialiasedElement)
            .addFunction("setNotAntialiasedElements", &QCustomPlot::setNotAntialiasedElements)
            .addFunction("setNoAntialiasingOnDrag", &QCustomPlot::setNoAntialiasingOnDrag)
            .addFunction("setOpenGl", &QCustomPlot::setOpenGl)
            .addFunction("setPlottingHint", &QCustomPlot::setPlottingHint)
            .addFunction("setPlottingHints", &QCustomPlot::setPlottingHints)
            .addFunction("setViewport", &QCustomPlot::setViewport)
            .addFunction("toPixmap", &QCustomPlot::toPixmap)
            .addFunction("viewport", &QCustomPlot::viewport)

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
              .addFunction("createAxisTickerPi", &LuaPlot::createAxisTickerPi)
              .addFunction("createAxisTickerText", &LuaPlot::createAxisTickerText)
              .addFunction("createAxisTickerTime", &LuaPlot::createAxisTickerTime)
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
              .addFunction("createTextElement", &LuaPlot::createTextElement)
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

template<typename T>
struct CoordHelper
{
    static int cellToCoord(lua_State* L)
    {
        // from lua: key, value = cellToCoord(colorMapData, keyIndex, valueIndex)

        int valueIndex = luabridge::Stack<int>::get(L, 3);
        int keyIndex = luabridge::Stack<int>::get(L, 2);
        T* cc = luabridge::Stack<T*>::get(L, 1);
        lua_settop(L, 0);

        double key = 0.0, value = 0.0;
        cc->cellToCoord(keyIndex, valueIndex, &key, &value);

        luabridge::Stack<double>::push(L, key);
        luabridge::Stack<double>::push(L, value);

        return 2;
    }

    static int coordToCell(lua_State* L)
    {
        // from lua: keyIndex, valueIndex = coordToCell(colorMapData, key, value)

        double value = luabridge::Stack<double>::get(L, 3);
        double key = luabridge::Stack<double>::get(L, 2);
        T* cc = luabridge::Stack<T*>::get(L, 1);
        lua_settop(L, 0);

        int keyIndex = 0, valueIndex = 0;
        cc->coordToCell(key, value, &keyIndex, &valueIndex);

        luabridge::Stack<double>::push(L, keyIndex);
        luabridge::Stack<double>::push(L, valueIndex);

        return 2;
    }

    static int pixelsToCoords(lua_State* L)
    {
        // from lua: key, value = pixelsToCoords(AbstractPlottable, x, y)

        double y = luabridge::Stack<double>::get(L, 3);
        double x = luabridge::Stack<double>::get(L, 2);
        T* cc = luabridge::Stack<T*>::get(L, 1);
        lua_settop(L, 0);

        double key = 0.0, value = 0.0;
        cc->pixelsToCoords(x, y, key, value);

        luabridge::Stack<double>::push(L, key);
        luabridge::Stack<double>::push(L, value);

        return 2;
    }

    static int coordsToPixels(lua_State* L)
    {
        // from lua: x, y = coordsToPixels(AbstractPlottable, key, value)

        double value = luabridge::Stack<double>::get(L, 3);
        double key = luabridge::Stack<double>::get(L, 2);
        T* cc = luabridge::Stack<T*>::get(L, 1);
        lua_settop(L, 0);

        double x = 0, y = 0;
        cc->coordsToPixels(key, value, x, y);

        luabridge::Stack<double>::push(L, x);
        luabridge::Stack<double>::push(L, y);

        return 2;
    }
};

static void QcpContainer2Lua(lua_State* L, const char* ns)
{
// begin of #define
#define CONTAINER_FUNCTIONS(c, d) \
    .addFunction("autoSqueeze", &c::autoSqueeze) \
    .addFunction("addContainer", static_cast<void(c::*)(const c&)>(&c::add)) \
    .addFunction("addVector", static_cast<void(c::*)(const QVector<d>&, bool)>(&c::add)) \
    .addFunction("addData", static_cast<void(c::*)(const d&)>(&c::add)) \
    .addFunction("clear", &c::clear) \
    .addFunction("isEmpty", &c::isEmpty) \
    .addFunction("remove", static_cast<void(c::*)(double)>(&c::remove)) \
    .addFunction("removeBetween", static_cast<void(c::*)(double, double)>(&c::remove)) \
    .addFunction("removeAfter", &c::removeAfter) \
    .addFunction("removeBefore", &c::removeBefore) \
    .addFunction("size", &c::size) \
    .addFunction("setAutoSqueeze", &c::setAutoSqueeze) \
    .addFunction("setVector", static_cast<void(c::*)(const QVector<d>&, bool)>(&c::set)) \
    .addFunction("setContainer", static_cast<void(c::*)(const c&)>(&c::set)) \
    .addFunction("sort", &c::sort) \
    .addFunction("squeeze", &c::squeeze)
// end of #define

    luabridge::getGlobalNamespace(L)
      .beginNamespace(ns)

        .beginClass<QCPColorMapData>("ColorMapData")
          .addConstructor<void(*)(int, int, const QCPRange&, const QCPRange&)>()
          .addFunction("alpha", &QCPColorMapData::alpha)
          .addFunction("cell", &QCPColorMapData::cell)
          .addFunction("clear", &QCPColorMapData::clear)
          .addFunction("clearAlpha", &QCPColorMapData::clearAlpha)
          .addFunction("data", &QCPColorMapData::data)
          .addFunction("dataBounds", &QCPColorMapData::dataBounds)
          .addFunction("fill", &QCPColorMapData::fill)
          .addFunction("fillAlpha", &QCPColorMapData::fillAlpha)
          .addFunction("isEmpty", &QCPColorMapData::isEmpty)
          .addFunction("keyRange", &QCPColorMapData::keyRange)
          .addFunction("keySize", &QCPColorMapData::keySize)
          .addFunction("recalculateDataBounds", &QCPColorMapData::recalculateDataBounds)
          .addFunction("setAlpha", &QCPColorMapData::setAlpha)
          .addFunction("setCell", &QCPColorMapData::setCell)
          .addFunction("setData", &QCPColorMapData::setData)
          .addFunction("setKeyRange", &QCPColorMapData::setKeyRange)
          .addFunction("setKeySize", &QCPColorMapData::setKeySize)
          .addFunction("setRange", &QCPColorMapData::setRange)
          .addFunction("setSize", &QCPColorMapData::setSize)
          .addFunction("setValueRange", &QCPColorMapData::setValueRange)
          .addFunction("setValueSize", &QCPColorMapData::setValueSize)
          .addFunction("valueRange", &QCPColorMapData::valueRange)
          .addFunction("valueSize", &QCPColorMapData::valueSize)
        .endClass()
        .beginNamespace("ColorMapDataHelper")
          .addCFunction("cellToCoord", &CoordHelper<QCPColorMapData>::cellToCoord)
          .addCFunction("coordToCell", &CoordHelper<QCPColorMapData>::coordToCell)
        .endNamespace()

        .beginClass<QCPBarsDataContainer>("BarsDataContainer")
            CONTAINER_FUNCTIONS(QCPBarsDataContainer, QCPBarsData)
        .endClass()

        .beginClass<QCPCurveDataContainer>("CurveDataContainer")
            CONTAINER_FUNCTIONS(QCPCurveDataContainer, QCPCurveData)
        .endClass()

        .beginClass<QCPFinancialDataContainer>("FinancialDataContainer")
            CONTAINER_FUNCTIONS(QCPFinancialDataContainer, QCPFinancialData)
        .endClass()

        .beginClass<QCPGraphDataContainer>("GraphDataContainer")
            CONTAINER_FUNCTIONS(QCPGraphDataContainer, QCPGraphData)
        .endClass()

        .beginClass<QCPStatisticalBoxDataContainer>("StatisticalBoxDataContainer")
            CONTAINER_FUNCTIONS(QCPStatisticalBoxDataContainer, QCPStatisticalBoxData)
        .endClass()

      .endNamespace()
    ;
#undef CONTAINER_FUNCTIONS
}

static void QcpBasic2Lua(lua_State* L, const char* ns)
{
    luabridge::getGlobalNamespace(L)
      .beginNamespace(ns)

        .beginClass<QCPAxisTicker>("AxisTicker")
          .addFunction("tickCount", &QCPAxisTicker::tickCount)
          .addFunction("tickOrigin", &QCPAxisTicker::tickOrigin)
          .addFunction("tickStepStrategy", &QCPAxisTicker::tickStepStrategy)
          .addFunction("setTickCount", &QCPAxisTicker::setTickCount)
          .addFunction("setTickOrigin", &QCPAxisTicker::setTickOrigin)
          .addFunction("setTickStepStrategy", &QCPAxisTicker::setTickStepStrategy)
        .endClass()   

          .deriveClass<QCPAxisTickerDateTime, QCPAxisTicker>("AxisTickerDateTime")
            .addStaticFunction("dateTimeToKey", static_cast<double(*)(const QDateTime)>(&QCPAxisTickerDateTime::dateTimeToKey))
            .addStaticFunction("dateToKey", static_cast<double(*)(const QDate)>(&QCPAxisTickerDateTime::dateTimeToKey))
            .addStaticFunction("keyToDateTime", &QCPAxisTickerDateTime::keyToDateTime)
            .addFunction("dateTimeFormat", &QCPAxisTickerDateTime::dateTimeFormat)
            .addFunction("dateTimeSpec", &QCPAxisTickerDateTime::dateTimeSpec)
            .addFunction("setDateTimeFormat", &QCPAxisTickerDateTime::setDateTimeFormat)
            .addFunction("setDateTimeSpec", &QCPAxisTickerDateTime::setDateTimeSpec)
            .addFunction("setTickOrigin", static_cast<void(QCPAxisTickerDateTime::*)(const QDateTime&)>(&QCPAxisTickerDateTime::setTickOrigin))
            .addFunction("setTickOriginByDouble", static_cast<void(QCPAxisTickerDateTime::*)(double)>(&QCPAxisTickerDateTime::setTickOrigin))
          .endClass()

          .deriveClass<QCPAxisTickerFixed, QCPAxisTicker>("AxisTickerFixed")
            .addFunction("scaleStrategy", &QCPAxisTickerFixed::scaleStrategy)
            .addFunction("setScaleStrategy", &QCPAxisTickerFixed::setScaleStrategy)
            .addFunction("setTickStep", &QCPAxisTickerFixed::setTickStep)
            .addFunction("tickStep", &QCPAxisTickerFixed::tickStep)
          .endClass()

          .deriveClass<QCPAxisTickerLog, QCPAxisTicker>("AxisTickerLog")
            .addFunction("logBase", &QCPAxisTickerLog::logBase)
            .addFunction("setLogBase", &QCPAxisTickerLog::setLogBase)
            .addFunction("setSubTickCount", &QCPAxisTickerLog::setSubTickCount)
            .addFunction("subTickCount", &QCPAxisTickerLog::subTickCount)
          .endClass()

          .deriveClass<QCPAxisTickerPi, QCPAxisTicker>("AxisTickerPi")
            .addFunction("fractionStyle", &QCPAxisTickerPi::fractionStyle)
            .addFunction("periodicity", &QCPAxisTickerPi::periodicity)
            .addFunction("piSymbol", &QCPAxisTickerPi::piSymbol)
            .addFunction("piValue", &QCPAxisTickerPi::piValue)
            .addFunction("setFractionStyle", &QCPAxisTickerPi::setFractionStyle)
            .addFunction("setPeriodicity", &QCPAxisTickerPi::setPeriodicity)
            .addFunction("setPiSymbol", &QCPAxisTickerPi::setPiSymbol)
            .addFunction("setPiValue", &QCPAxisTickerPi::setPiValue)
          .endClass()

          .deriveClass<QCPAxisTickerText, QCPAxisTicker>("AxisTickerText")
            .addFunction("addTick", &QCPAxisTickerText::addTick)
            .addFunction("addTicksByMap", static_cast<void(QCPAxisTickerText::*)(const QMap<double, QString>&)>(&QCPAxisTickerText::addTicks))
            .addFunction("addTicksByVector", static_cast<void(QCPAxisTickerText::*)(const QVector<double>&, const QVector<QString>&)>(&QCPAxisTickerText::addTicks))
            .addFunction("clear", &QCPAxisTickerText::clear)
            .addFunction("setSubTickCount", &QCPAxisTickerText::setSubTickCount)
            .addFunction("setTicksByMap",  static_cast<void(QCPAxisTickerText::*)(const QMap<double, QString>&)>(&QCPAxisTickerText::setTicks))
            .addFunction("setTicksByVector", static_cast<void(QCPAxisTickerText::*)(const QVector<double>&, const QVector<QString>&)>(&QCPAxisTickerText::setTicks))
            .addFunction("subTickCount", &QCPAxisTickerText::subTickCount)
            .addFunction("ticks", &QCPAxisTickerText::ticks)
          .endClass()

          .deriveClass<QCPAxisTickerTime, QCPAxisTicker>("AxisTickerTime")
            .addFunction("fieldWidth", &QCPAxisTickerTime::fieldWidth)
            .addFunction("setFieldWidth", &QCPAxisTickerTime::setFieldWidth)
            .addFunction("setTimeFormat", &QCPAxisTickerTime::setTimeFormat)
            .addFunction("timeFormat", &QCPAxisTickerTime::timeFormat)
          .endClass()

        .deriveClass<QCPBarsGroup, QObject>("BarsGroup")
          .addFunction("append", &QCPBarsGroup::append)
          .addFunction("bars", static_cast<QList<QCPBars*>(QCPBarsGroup::*)()const>(&QCPBarsGroup::bars))
          .addFunction("barByIndex", static_cast<QCPBars*(QCPBarsGroup::*)(int)const>(&QCPBarsGroup::bars))
          .addFunction("clear", &QCPBarsGroup::clear)
          .addFunction("contains", &QCPBarsGroup::contains)
          .addFunction("insert", &QCPBarsGroup::insert)
          .addFunction("isEmpty", &QCPBarsGroup::isEmpty)
          .addFunction("remove", &QCPBarsGroup::remove)
          .addFunction("setSpacingType", &QCPBarsGroup::setSpacingType)
          .addFunction("setSpacing", &QCPBarsGroup::setSpacing)
          .addFunction("size", &QCPBarsGroup::size)
          .addFunction("spacing", &QCPBarsGroup::spacing)
          .addFunction("spacingType", &QCPBarsGroup::spacingType)
        .endClass()

        .beginClass<QCPColorGradient>("ColorGradient")
          .addConstructor<void(*)(QCPColorGradient::GradientPreset)>()
          .addFunction("clearColorStops", &QCPColorGradient::clearColorStops)
          .addFunction("color", &QCPColorGradient::color)
          .addFunction("colorInterpolation", &QCPColorGradient::colorInterpolation)
          .addFunction("colorStops", &QCPColorGradient::colorStops)
          .addFunction("colorize1", static_cast<void(QCPColorGradient::*)(const double*, const unsigned char*, const QCPRange&, QRgb*, int, int, bool)>(&QCPColorGradient::colorize))
          .addFunction("colorize2", static_cast<void(QCPColorGradient::*)(const double*, const QCPRange&, QRgb*, int, int, bool)>(&QCPColorGradient::colorize))
          .addFunction("inverted", &QCPColorGradient::inverted)
          .addFunction("levelCount", &QCPColorGradient::levelCount)
          .addFunction("loadPreset", &QCPColorGradient::loadPreset)
          .addFunction("periodic", &QCPColorGradient::periodic)
          .addFunction("setColorStops", &QCPColorGradient::setColorStops)
          .addFunction("setColorStopAt", &QCPColorGradient::setColorStopAt)
          .addFunction("setColorInterpolation", &QCPColorGradient::setColorInterpolation)
          .addFunction("setLevelCount", &QCPColorGradient::setLevelCount)
          .addFunction("setPeriodic", &QCPColorGradient::setPeriodic)
        .endClass()

//        .beginClass<QCPDataSelection>("DataSelection")
//        .endClass()

        .beginClass<QCPItemAnchor>("ItemAnchor")
          .addFunction("name", &QCPItemAnchor::name)
          .addFunction("pixelPosition", &QCPItemAnchor::pixelPosition)
        .endClass()

          .deriveClass<QCPItemPosition, QCPItemAnchor>("ItemPosition")
            .addFunction("axisRect", &QCPItemPosition::axisRect)
            .addFunction("coords", &QCPItemPosition::coords)
            .addFunction("key", &QCPItemPosition::key)
            .addFunction("keyAxis", &QCPItemPosition::keyAxis)
            .addFunction("parentAnchor", &QCPItemPosition::parentAnchor)
            .addFunction("parentAnchorX", &QCPItemPosition::parentAnchorX)
            .addFunction("parentAnchorY", &QCPItemPosition::parentAnchorY)
            .addFunction("pixelPosition", &QCPItemPosition::pixelPosition)
            .addFunction("type", &QCPItemPosition::type)
            .addFunction("typeX", &QCPItemPosition::typeX)
            .addFunction("typeY", &QCPItemPosition::typeY)
            .addFunction("value", &QCPItemPosition::value)
            .addFunction("valueAxis", &QCPItemPosition::valueAxis)
            .addFunction("setAxes", &QCPItemPosition::setAxes)
            .addFunction("setAxisRect", &QCPItemPosition::setAxisRect)
            .addFunction("setCoordsXY", static_cast<void(QCPItemPosition::*)(double, double)>(&QCPItemPosition::setCoords))
            .addFunction("setCoords", static_cast<void(QCPItemPosition::*)(const QPointF&)>(&QCPItemPosition::setCoords))
            .addFunction("setParentAnchor", &QCPItemPosition::setParentAnchor)
            .addFunction("setParentAnchorX", &QCPItemPosition::setParentAnchorX)
            .addFunction("setParentAnchorY", &QCPItemPosition::setParentAnchorY)
            .addFunction("setPixelPosition", &QCPItemPosition::setPixelPosition)
            .addFunction("setType", &QCPItemPosition::setType)
            .addFunction("setTypeX", &QCPItemPosition::setTypeX)
            .addFunction("setTypeY", &QCPItemPosition::setTypeY)
          .endClass()

        .deriveClass<QCPLayer, QObject>("Layer")
          .addFunction("children", &QCPLayer::children)
          .addFunction("index", &QCPLayer::index)
          .addFunction("parentPlot", &QCPLayer::parentPlot)
          .addFunction("mode", &QCPLayer::mode)
          .addFunction("name", &QCPLayer::name)
          .addFunction("replot", &QCPLayer::replot)
          .addFunction("setVisible", &QCPLayer::setVisible)
          .addFunction("setMode", &QCPLayer::setMode)
          .addFunction("visible", &QCPLayer::visible)
        .endClass()

        .beginClass<QCPLineEnding>("LineEnding")
          .addConstructor<void(*)(QCPLineEnding::EndingStyle, double, double, bool)>()
          .addFunction("boundingDistance", &QCPLineEnding::boundingDistance)
          .addFunction("inverted", &QCPLineEnding::inverted)
          .addFunction("length", &QCPLineEnding::length)
          .addFunction("realLength", &QCPLineEnding::realLength)
          .addFunction("setStyle", &QCPLineEnding::setStyle)
          .addFunction("setWidth", &QCPLineEnding::setWidth)
          .addFunction("setInverted", &QCPLineEnding::setInverted)
          .addFunction("setLength", &QCPLineEnding::setLength)
          .addFunction("style", &QCPLineEnding::style)
          .addFunction("width", &QCPLineEnding::width)
        .endClass()

        .deriveClass<QCPMarginGroup, QObject>("MarginGroup")
          .addConstructor<void(*)(QCustomPlot*)>()
          .addFunction("clear", &QCPMarginGroup::clear)
          .addFunction("elements", &QCPMarginGroup::elements)
          .addFunction("isEmpty", &QCPMarginGroup::isEmpty)
        .endClass()

        .beginClass<QCPRange>("Range")
          .addConstructor<void(*)(double, double)>()
          .addData("upper", &QCPRange::upper)
          .addData("lower", &QCPRange::lower)
          .addFunction("bounded", &QCPRange::bounded)
          .addFunction("center", &QCPRange::center)
          .addFunction("contains", &QCPRange::contains)
          .addFunction("expand", static_cast<void(QCPRange::*)(double)>(&QCPRange::expand))
          .addFunction("expandByRange", static_cast<void(QCPRange::*)(const QCPRange&)>(&QCPRange::expand))
          .addFunction("expanded", static_cast<QCPRange(QCPRange::*)(double)const>(&QCPRange::expanded))
          .addFunction("expandedByRange", static_cast<QCPRange(QCPRange::*)(const QCPRange&)const>(&QCPRange::expanded))
          .addFunction("normalize", &QCPRange::normalize)
          .addFunction("sanitizedForLinScale", &QCPRange::sanitizedForLinScale)
          .addFunction("sanitizedForLogScale", &QCPRange::sanitizedForLogScale)
          .addFunction("size", &QCPRange::size)
        .endClass()

        .beginClass<QCPScatterStyle>("ScatterStyle")
          .addFunction("brush", &QCPScatterStyle::brush)
          .addFunction("customPath", &QCPScatterStyle::customPath)
          .addFunction("pen", &QCPScatterStyle::pen)
          .addFunction("pixmap", &QCPScatterStyle::pixmap)
          .addFunction("isNone", &QCPScatterStyle::isNone)
          .addFunction("isPenDefined", &QCPScatterStyle::isPenDefined)
          .addFunction("setBrush", &QCPScatterStyle::setBrush)
          .addFunction("setCustomPath", &QCPScatterStyle::setCustomPath)
          .addFunction("setFromOther", &QCPScatterStyle::setFromOther)
          .addFunction("setPen", &QCPScatterStyle::setPen)
          .addFunction("setPixmap", &QCPScatterStyle::setPixmap)
          .addFunction("setSize", &QCPScatterStyle::setSize)
          .addFunction("setShape", &QCPScatterStyle::setShape)
          .addFunction("shape", &QCPScatterStyle::shape)
          .addFunction("size", &QCPScatterStyle::size)
          .addFunction("undefinePen", &QCPScatterStyle::undefinePen)
        .endClass()
        .beginClass<ScatterStyleConstructor>("ScatterStyleConstructor")
          .addStaticFunction("fromShapeAndSize", &ScatterStyleConstructor::fromShapeAndSize)
          .addStaticFunction("fromShapePenBrushAndSize", &ScatterStyleConstructor::fromShapePenBrushAndSize)
          .addStaticFunction("fromPainterPath", &ScatterStyleConstructor::fromPainterPath)
          .addStaticFunction("fromPixmap", &ScatterStyleConstructor::fromPixmap)
        .endClass()
/*
        .beginClass<QCPSelectionDecorator>("QCPSelectionDecorator")
        .endClass()

          .deriveClass<QCPSelectionDecoratorBracket, QCPSelectionDecorator>("QCPSelectionDecoratorBracket")
          .endClass()

        .beginClass<QCPVector2D>("QCPVector2D")
        .endClass()
*/
      .endNamespace()
    ;
}

static void QcpLayerable2Lua(lua_State* L, const char* ns)
{
    luabridge::getGlobalNamespace(L)
      .beginNamespace(ns)

        .deriveClass<QCPLayerable, QObject>("Layerable")
          .addFunction("antialiased", &QCPLayerable::antialiased)
          .addFunction("parentLayerable", &QCPLayerable::parentLayerable)
          .addFunction("parentPlot", &QCPLayerable::parentPlot)
          .addFunction("realVisibility", &QCPLayerable::realVisibility)
          .addFunction("setAntialiased", &QCPLayerable::setAntialiased)
          .addFunction("setVisible", &QCPLayerable::setVisible)
          .addFunction("setLayer", static_cast<bool(QCPLayerable::*)(QCPLayer*)>(&QCPLayerable::setLayer))
          .addFunction("setLayerByName", static_cast<bool(QCPLayerable::*)(const QString&)>(&QCPLayerable::setLayer))
          .addFunction("visible", &QCPLayerable::visible)
        .endClass()

          .deriveClass<QCPAxis, QCPLayerable>("Axis")
            .addStaticFunction("orientation", static_cast<Qt::Orientation(*)(QCPAxis::AxisType)>(&QCPAxis::orientation))

            .addFunction("axisRect", &QCPAxis::axisRect)
            .addFunction("axisType", &QCPAxis::axisType)
            .addFunction("basePen", &QCPAxis::basePen)
            .addFunction("coordToPixel", &QCPAxis::coordToPixel)
            .addFunction("grid", &QCPAxis::grid)
            .addFunction("graphs", &QCPAxis::graphs)
            .addFunction("items", &QCPAxis::items)
            .addFunction("label", &QCPAxis::label)
            .addFunction("labelPadding", &QCPAxis::labelPadding)
            .addFunction("labelFont", &QCPAxis::labelFont)
            .addFunction("labelColor", &QCPAxis::labelColor)
            .addFunction("lowerEnding", &QCPAxis::lowerEnding)
            .addFunction("moveRange", &QCPAxis::moveRange)
            .addFunction("numberFormat", &QCPAxis::numberFormat)
            .addFunction("numberPrecision", &QCPAxis::numberPrecision)
            .addFunction("offset", &QCPAxis::offset)
            .addFunction("orientation", static_cast<Qt::Orientation(QCPAxis::*)()const>(&QCPAxis::orientation))
            .addFunction("padding", &QCPAxis::padding)
            .addFunction("pixelToCoord", &QCPAxis::pixelToCoord)
            .addFunction("pixelOrientation", &QCPAxis::pixelOrientation)
            .addFunction("plottables", &QCPAxis::plottables)
            .addFunction("range", &QCPAxis::range)
            .addFunction("rangeReversed", &QCPAxis::rangeReversed)
            .addFunction("rescale", &QCPAxis::rescale)
            .addFunction("scaleRange", static_cast<void(QCPAxis::*)(double, double)>(&QCPAxis::scaleRange))
            .addFunction("scaleType", &QCPAxis::scaleType)
            .addFunction("setBasePen", &QCPAxis::setBasePen)
            .addFunction("setScaleRatio", &QCPAxis::setScaleRatio)
            .addFunction("setLowerEnding", &QCPAxis::setLowerEnding)
            .addFunction("setUpperEnding", &QCPAxis::setUpperEnding)
            .addFunction("setPadding", &QCPAxis::setPadding)
            .addFunction("setRange", static_cast<void(QCPAxis::*)(double, double)>(&QCPAxis::setRange))
            .addFunction("setRangeLower", &QCPAxis::setRangeLower)
            .addFunction("setRangeReversed", &QCPAxis::setRangeReversed)
            .addFunction("setRangeWithSize", static_cast<void(QCPAxis::*)(double, double, Qt::AlignmentFlag)>(&QCPAxis::setRange))
            .addFunction("setRangeUpper", &QCPAxis::setRangeUpper)
            .addFunction("setLabel", &QCPAxis::setLabel)
            .addFunction("setLabelFont", &QCPAxis::setLabelFont)
            .addFunction("setLabelColor", &QCPAxis::setLabelColor)
            .addFunction("setLabelPadding", &QCPAxis::setLabelPadding)
            .addFunction("setNumberFormat", &QCPAxis::setNumberFormat)
            .addFunction("setNumberPrecision", &QCPAxis::setNumberPrecision)
            .addFunction("setPadding", &QCPAxis::setPadding)
            .addFunction("setOffset", &QCPAxis::setOffset)
            .addFunction("setScaleType", &QCPAxis::setScaleType)
            .addFunction("setSubTickLength", &QCPAxis::setSubTickLength)
            .addFunction("setSubTickPen", &QCPAxis::setSubTickPen)
            .addFunction("setSubTicks", &QCPAxis::setSubTicks)
            .addFunction("setSubTickLength", &QCPAxis::setSubTickLength)
            .addFunction("setSubTickLengthIn", &QCPAxis::setSubTickLengthIn)
            .addFunction("setSubTickLengthOut", &QCPAxis::setSubTickLengthOut)
            .addFunction("setTicker", &QCPAxis::setTicker)
            .addFunction("setTicks", &QCPAxis::setTicks)
            .addFunction("setTickLabels", &QCPAxis::setTickLabels)
            .addFunction("setTickLabelColor", &QCPAxis::setTickLabelColor)
            .addFunction("setTickLabelFont", &QCPAxis::setTickLabelFont)
            .addFunction("setTickLabelPadding", &QCPAxis::setTickLabelPadding)
            .addFunction("setTickLabelRotation", &QCPAxis::setTickLabelRotation)
            .addFunction("setTickLabelSide", &QCPAxis::setTickLabelSide)
            .addFunction("setTickLength", &QCPAxis::setTickLength)
            .addFunction("setTickLengthIn", &QCPAxis::setTickLengthIn)
            .addFunction("setTickLengthOut", &QCPAxis::setTickLengthOut)
            .addFunction("setTickPen", &QCPAxis::setTickPen)
            .addFunction("setTicks", &QCPAxis::setTicks)
            .addFunction("setUpperEnding", &QCPAxis::setUpperEnding)
            .addFunction("subTickLengthIn", &QCPAxis::subTickLengthIn)
            .addFunction("subTickLengthOut", &QCPAxis::subTickLengthOut)
            .addFunction("ticker", &QCPAxis::ticker)
            .addFunction("tickLabelColor", &QCPAxis::tickLabelColor)
            .addFunction("tickLabelFont", &QCPAxis::tickLabelFont)
            .addFunction("tickLabelPadding", &QCPAxis::tickLabelPadding)
            .addFunction("tickLabelRotation", &QCPAxis::tickLabelRotation)
            .addFunction("tickLabels", &QCPAxis::tickLabels)
            .addFunction("tickLabelSide", &QCPAxis::tickLabelSide)
            .addFunction("tickLengthIn", &QCPAxis::tickLengthIn)
            .addFunction("tickLengthOut", &QCPAxis::tickLengthOut)
            .addFunction("tickPen", &QCPAxis::tickPen)
            .addFunction("ticks", &QCPAxis::ticks)
            .addFunction("tickVector", &QCPAxis::tickVector)
            .addFunction("tickVectorLabels", &QCPAxis::tickVectorLabels)
            .addFunction("upperEnding", &QCPAxis::upperEnding)
          .endClass()

          .deriveClass<QCPGrid, QCPLayerable>("Grid")
            .addFunction("antialiasedSubGrid", &QCPGrid::antialiasedSubGrid)
            .addFunction("antialiasedZeroLine", &QCPGrid::antialiasedZeroLine)
            .addFunction("pen", &QCPGrid::pen)
            .addFunction("setAntialiasedSubGrid", &QCPGrid::setAntialiasedSubGrid)
            .addFunction("setAntialiasedZeroLine", &QCPGrid::setAntialiasedZeroLine)
            .addFunction("setPen", &QCPGrid::setPen)
            .addFunction("setSubGridPen", &QCPGrid::setSubGridPen)
            .addFunction("setSubGridVisible", &QCPGrid::setSubGridVisible)
            .addFunction("setZeroLinePen", &QCPGrid::setZeroLinePen)
            .addFunction("subGridPen", &QCPGrid::subGridPen)
            .addFunction("subGridVisible", &QCPGrid::subGridVisible)
            .addFunction("zeroLinePen", &QCPGrid::zeroLinePen)
          .endClass()

//          .deriveClass<QCPSelectionRect, QCPLayerable>("SelectionRect")
//          .endClass()

      .endNamespace()
    ;
}

static void QcpItem2Lua(lua_State* L, const char* ns)
{
    luabridge::getGlobalNamespace(L)
      .beginNamespace(ns)

        .deriveClass<QCPAbstractItem, QCPLayerable>("AbstractItem")
          .addFunction("anchor", &QCPAbstractItem::anchor)
          .addFunction("anchors", &QCPAbstractItem::anchors)
          .addFunction("clipAxisRect", &QCPAbstractItem::clipAxisRect)
          .addFunction("clipToAxisRect", &QCPAbstractItem::clipToAxisRect)
          .addFunction("hasAnchor", &QCPAbstractItem::hasAnchor)
          .addFunction("position", &QCPAbstractItem::position)
          .addFunction("positions", &QCPAbstractItem::positions)
          .addFunction("setClipToAxisRect", &QCPAbstractItem::setClipToAxisRect)
          .addFunction("setClipAxisRect", &QCPAbstractItem::setClipAxisRect)
        .endClass()

          .deriveClass<QCPItemBracket, QCPAbstractItem>("ItemBracket")
            .addData("left", &QCPItemBracket::left)
            .addData("right", &QCPItemBracket::right)
            .addData("center", &QCPItemBracket::center)

            .addFunction("length", &QCPItemBracket::length)
            .addFunction("pen", &QCPItemBracket::pen)
            .addFunction("style", &QCPItemBracket::style)
            .addFunction("setLength", &QCPItemBracket::setLength)
            .addFunction("setPen", &QCPItemBracket::setPen)
            .addFunction("setStyle", &QCPItemBracket::setStyle)
          .endClass()

          .deriveClass<QCPItemCurve, QCPAbstractItem>("ItemCurve")
            .addData("endDir", &QCPItemCurve::endDir)
            .addData("start", &QCPItemCurve::start)
            .addData("startDir", &QCPItemCurve::startDir)
            .addData("theEnd", &QCPItemCurve::end)

            .addFunction("head", &QCPItemCurve::head)
            .addFunction("pen", &QCPItemCurve::pen)
            .addFunction("tail", &QCPItemCurve::tail)
            .addFunction("setHead", &QCPItemCurve::setHead)
            .addFunction("setPen", &QCPItemCurve::setPen)
            .addFunction("setTail", &QCPItemCurve::setTail)
          .endClass()

          .deriveClass<QCPItemEllipse, QCPAbstractItem>("ItemEllipse")
            .addData("topLeft", &QCPItemEllipse::topLeft)
            .addData("bottomRight", &QCPItemEllipse::bottomRight)
            .addData("topLeftRim", &QCPItemEllipse::topLeftRim)
            .addData("top", &QCPItemEllipse::top)
            .addData("topRightRim", &QCPItemEllipse::topRightRim)
            .addData("right", &QCPItemEllipse::right)
            .addData("bottomRightRim", &QCPItemEllipse::bottomRightRim)
            .addData("bottom", &QCPItemEllipse::bottom)
            .addData("bottomLeftRim", &QCPItemEllipse::bottomLeftRim)
            .addData("left", &QCPItemEllipse::left)
            .addData("center", &QCPItemEllipse::center)

            .addFunction("pen", &QCPItemEllipse::pen)
            .addFunction("brush", &QCPItemEllipse::brush)
            .addFunction("setPen", &QCPItemEllipse::setPen)
            .addFunction("setBrush", &QCPItemEllipse::setBrush)
          .endClass()

          .deriveClass<QCPItemLine, QCPAbstractItem>("ItemLine")
            .addData("start", &QCPItemLine::start)
            .addData("theEnd", &QCPItemLine::end)

            .addFunction("head", &QCPItemLine::head)
            .addFunction("pen", &QCPItemLine::pen)
            .addFunction("tail", &QCPItemLine::tail)
            .addFunction("setHead", &QCPItemLine::setHead)
            .addFunction("setPen", &QCPItemLine::setPen)
            .addFunction("setTail", &QCPItemLine::setTail)
          .endClass()

          .deriveClass<QCPItemPixmap, QCPAbstractItem>("ItemPixmap")
            .addData("topLeft", &QCPItemPixmap::topLeft)
            .addData("bottomRight", &QCPItemPixmap::bottomRight)
            .addData("top", &QCPItemPixmap::top)
            .addData("topRight", &QCPItemPixmap::topRight)
            .addData("right", &QCPItemPixmap::right)
            .addData("bottom", &QCPItemPixmap::bottom)
            .addData("bottomLeft", &QCPItemPixmap::bottomLeft)
            .addData("left", &QCPItemPixmap::left)

            .addFunction("aspectRatioMode", &QCPItemPixmap::aspectRatioMode)
            .addFunction("pen", &QCPItemPixmap::pen)
            .addFunction("pixmap", &QCPItemPixmap::pixmap)
            .addFunction("scaled", &QCPItemPixmap::scaled)
            .addFunction("setPen", &QCPItemPixmap::setPen)
            .addFunction("setPixmap", &QCPItemPixmap::setPixmap)
            .addFunction("setScaled", &QCPItemPixmap::setScaled)
            .addFunction("transformationMode", &QCPItemPixmap::transformationMode)
          .endClass()

          .deriveClass<QCPItemRect, QCPAbstractItem>("ItemRect")
            .addData("topLeft", &QCPItemRect::topLeft)
            .addData("bottomRight", &QCPItemRect::bottomRight)
            .addData("top", &QCPItemRect::top)
            .addData("topRight", &QCPItemRect::topRight)
            .addData("right", &QCPItemRect::right)
            .addData("bottom", &QCPItemRect::bottom)
            .addData("bottomLeft", &QCPItemRect::bottomLeft)
            .addData("left", &QCPItemRect::left)

            .addFunction("brush", &QCPItemRect::brush)
            .addFunction("pen", &QCPItemRect::pen)
            .addFunction("setBrush", &QCPItemRect::setBrush)
            .addFunction("setPen", &QCPItemRect::setPen)
          .endClass()

          .deriveClass<QCPItemStraightLine, QCPAbstractItem>("ItemStraightLine")
            .addData("point1", &QCPItemStraightLine::point1)
            .addData("point2", &QCPItemStraightLine::point2)

            .addFunction("pen", &QCPItemStraightLine::pen)
            .addFunction("setPen", &QCPItemStraightLine::setPen)
          .endClass()

          .deriveClass<QCPItemText, QCPAbstractItem>("ItemText")
            .addData("position", &QCPItemText::position)
            .addData("left", &QCPItemText::left)
            .addData("right", &QCPItemText::right)
            .addData("top", &QCPItemText::top)
            .addData("topLeft", &QCPItemText::topLeft)
            .addData("topRight", &QCPItemText::topRight)
            .addData("bottom", &QCPItemText::bottom)
            .addData("bottomLeft", &QCPItemText::bottomLeft)
            .addData("bottomRight", &QCPItemText::bottomRight)

            .addFunction("color", &QCPItemText::color)
            .addFunction("brush", &QCPItemText::brush)
            .addFunction("font", &QCPItemText::font)
            .addFunction("padding", &QCPItemText::padding)
            .addFunction("pen", &QCPItemText::pen)
            .addFunction("positionAlignment", &QCPItemText::positionAlignment)
            .addFunction("rotation", &QCPItemText::rotation)
            .addFunction("setColor", &QCPItemText::setColor)
            .addFunction("setBrush", &QCPItemText::setBrush)
            .addFunction("setPen", &QCPItemText::setPen)
            .addFunction("setPositionAlignment", &QCPItemText::setPositionAlignment)
            .addFunction("setText", &QCPItemText::setText)
            .addFunction("setTextAlignment", &QCPItemText::setTextAlignment)
            .addFunction("setFont", &QCPItemText::setFont)
            .addFunction("setPadding", &QCPItemText::setPadding)
            .addFunction("setRotation", &QCPItemText::setRotation)
            .addFunction("text", &QCPItemText::text)
            .addFunction("textAlignment", &QCPItemText::textAlignment)
          .endClass()

          .deriveClass<QCPItemTracer, QCPAbstractItem>("QCPItemTracer")
            .addData("position", &QCPItemTracer::position)

            .addFunction("brush", &QCPItemTracer::brush)
            .addFunction("graph", &QCPItemTracer::graph)
            .addFunction("graphKey", &QCPItemTracer::graphKey)
            .addFunction("interpolating", &QCPItemTracer::interpolating)
            .addFunction("pen", &QCPItemTracer::pen)
            .addFunction("size", &QCPItemTracer::size)
            .addFunction("style", &QCPItemTracer::style)
            .addFunction("updatePosition", &QCPItemTracer::updatePosition)
            .addFunction("setBrush", &QCPItemTracer::setBrush)
            .addFunction("setGraph", &QCPItemTracer::setGraph)
            .addFunction("setGraphKey", &QCPItemTracer::setGraphKey)
            .addFunction("setInterpolating", &QCPItemTracer::setInterpolating)
            .addFunction("setPen", &QCPItemTracer::setPen)
            .addFunction("setSize", &QCPItemTracer::setSize)
            .addFunction("setStyle", &QCPItemTracer::setStyle)
          .endClass()

      .endNamespace()
    ;
}

static void QcpPlottable2Lua(lua_State* L, const char* ns)
{
// begin of #define ABSTRACT_PLOTTABLE_1D_FUNCTIONS
#define ABSTRACT_PLOTTABLE_1D_FUNCTIONS(c) \
    .addFunction("dataCount", &c::dataCount) \
    .addFunction("dataMainKey", &c::dataMainKey) \
    .addFunction("dataPixelPosition", &c::dataPixelPosition) \
    .addFunction("dataSortKey", &c::dataSortKey) \
    .addFunction("dataMainValue", &c::dataMainValue) \
    .addFunction("dataValueRange", &c::dataValueRange) \
    .addFunction("sortKeyIsMainKey", &c::sortKeyIsMainKey) \
    .addFunction("findBegin", &c::findBegin) \
    .addFunction("findEnd", &c::findEnd)
// end of #define ABSTRACT_PLOTTABLE_1D_FUNCTIONS

    luabridge::getGlobalNamespace(L)
      .beginNamespace(ns)

        .deriveClass<QCPAbstractPlottable, QCPLayerable>("AbstractPlottable")
          .addFunction("addToLegend", static_cast<bool(QCPAbstractPlottable::*)(QCPLegend*)>(&QCPAbstractPlottable::addToLegend))
          .addFunction("addLegendToLegend", static_cast<bool(QCPAbstractPlottable::*)()>(&QCPAbstractPlottable::addToLegend))
          .addFunction("antialiasedFill", &QCPAbstractPlottable::antialiasedFill)
          .addFunction("antialiasedScatters", &QCPAbstractPlottable::antialiasedScatters)
          .addFunction("brush", &QCPAbstractPlottable::brush)
          .addFunction("coordsToPixels", static_cast<QPointF(QCPAbstractPlottable::*)(double, double)const>(&QCPAbstractPlottable::coordsToPixels))
          .addFunction("getKeyRange", &QCPAbstractPlottable::getKeyRange)
          .addFunction("getValueRange", &QCPAbstractPlottable::getValueRange)
          .addFunction("keyAxis", &QCPAbstractPlottable::keyAxis)
          .addFunction("name", &QCPAbstractPlottable::name)
          .addFunction("pen", &QCPAbstractPlottable::pen)
          .addFunction("valueAxis", &QCPAbstractPlottable::valueAxis)
          .addFunction("removeFromLegend", static_cast<bool(QCPAbstractPlottable::*)()const>(&QCPAbstractPlottable::removeFromLegend))
          .addFunction("removeLegendFromLegend", static_cast<bool(QCPAbstractPlottable::*)(QCPLegend*)const>(&QCPAbstractPlottable::removeFromLegend))
          .addFunction("rescaleAxes", &QCPAbstractPlottable::rescaleAxes)
          .addFunction("rescaleKeyAxis", &QCPAbstractPlottable::rescaleKeyAxis)
          .addFunction("setAntialiasedFill", &QCPAbstractPlottable::setAntialiasedFill)
          .addFunction("setAntialiasedScatters", &QCPAbstractPlottable::setAntialiasedScatters)
          .addFunction("setBrush", &QCPAbstractPlottable::setBrush)
          .addFunction("setKeyAxis", &QCPAbstractPlottable::setKeyAxis)
          .addFunction("setName", &QCPAbstractPlottable::setName)
          .addFunction("setValueAxis", &QCPAbstractPlottable::setValueAxis)
          .addFunction("setPen", &QCPAbstractPlottable::setPen)
          .addFunction("valueAxis", &QCPAbstractPlottable::valueAxis)
        .endClass()
        .beginNamespace("AbstractPlottableHelper")
          .addCFunction("pixelsToCoords", &CoordHelper<QCPAbstractPlottable>::pixelsToCoords)
          .addCFunction("coordsToPixels", &CoordHelper<QCPAbstractPlottable>::coordsToPixels)
        .endNamespace()

          .deriveClass<QCPColorMap, QCPAbstractPlottable>("ColorMap")
            .addFunction("colorScale", &QCPColorMap::colorScale)
            .addFunction("data", &QCPColorMap::data)
            .addFunction("dataRange", &QCPColorMap::dataRange)
            .addFunction("dataScaleType", &QCPColorMap::dataScaleType)
            .addFunction("gradient", &QCPColorMap::gradient)
            .addFunction("getKeyRange", &QCPColorMap::getKeyRange)
            .addFunction("getValueRange", &QCPColorMap::getValueRange)
            .addFunction("interpolate", &QCPColorMap::interpolate)
            .addFunction("rescaleAxes", &QCPColorMap::rescaleAxes)
            .addFunction("rescaleDataRange", &QCPColorMap::rescaleDataRange)
            .addFunction("setColorScale", &QCPColorMap::setColorScale)
            .addFunction("setData", &QCPColorMap::setData)
            .addFunction("setDataRange", &QCPColorMap::setDataRange)
            .addFunction("setDataScaleType", &QCPColorMap::setDataScaleType)
            .addFunction("setInterpolate", &QCPColorMap::setInterpolate)
            .addFunction("setGradient", &QCPColorMap::setGradient)
            .addFunction("setTightBoundary", &QCPColorMap::setTightBoundary)
            .addFunction("tightBoundary", &QCPColorMap::tightBoundary)
            .addFunction("updateLegendIcon", &QCPColorMap::updateLegendIcon)
          .endClass()

          .deriveClass<QCPErrorBars, QCPAbstractPlottable>("ErrorBars")
             ABSTRACT_PLOTTABLE_1D_FUNCTIONS(QCPErrorBars)
            .addFunction("addData1", static_cast<void(QCPErrorBars::*)(double)>(&QCPErrorBars::addData))
            .addFunction("addData2", static_cast<void(QCPErrorBars::*)(double, double)>(&QCPErrorBars::addData))
            .addFunction("addVector1", static_cast<void(QCPErrorBars::*)(const QVector<double>&)>(&QCPErrorBars::addData))
            .addFunction("addVector2", static_cast<void(QCPErrorBars::*)(const QVector<double>&, const QVector<double>&)>(&QCPErrorBars::addData))
            .addFunction("data", &QCPErrorBars::data)
            .addFunction("dataPlottable", &QCPErrorBars::dataPlottable)
            .addFunction("errorType", &QCPErrorBars::errorType)
            .addFunction("symbolGap", &QCPErrorBars::symbolGap)
            .addFunction("whiskerWidth", &QCPErrorBars::whiskerWidth)
            .addFunction("setContainer", static_cast<void (QCPErrorBars::*)(QSharedPointer<QCPErrorBarsDataContainer>)>(&QCPErrorBars::setData))
            .addFunction("setDataPlottable", &QCPErrorBars::setDataPlottable)
            .addFunction("setErrorType", &QCPErrorBars::setErrorType)
            .addFunction("setSymbolGap", &QCPErrorBars::setSymbolGap)
            .addFunction("setVector1", static_cast<void(QCPErrorBars::*)(const QVector<double>&)>(&QCPErrorBars::setData))
            .addFunction("setVector2", static_cast<void(QCPErrorBars::*)(const QVector<double>&, const QVector<double>&)>(&QCPErrorBars::setData))
            .addFunction("setWhiskerWidth", &QCPErrorBars::setWhiskerWidth)
          .endClass()

          .deriveClass<QCPAbstractPlottable1D<QCPBarsData>, QCPAbstractPlottable>("AbstractPlottable1D_BarsData")
            ABSTRACT_PLOTTABLE_1D_FUNCTIONS(QCPAbstractPlottable1D<QCPBarsData>)
          .endClass()
            .deriveClass<QCPBars, QCPAbstractPlottable1D<QCPBarsData> >("Bars")
              .addFunction("addData", static_cast<void(QCPBars::*)(double, double)>(&QCPBars::addData))
              .addFunction("addVector", static_cast<void(QCPBars::*)(const QVector<double>&, const QVector<double>&, bool)>(&QCPBars::addData))
              .addFunction("barAbove", &QCPBars::barAbove)
              .addFunction("barBelow", &QCPBars::barBelow)
              .addFunction("barsGroup", &QCPBars::barsGroup)
              .addFunction("baseValue", &QCPBars::baseValue)
              .addFunction("data", &QCPBars::data)
              .addFunction("getKeyRange", &QCPBars::getKeyRange)
              .addFunction("getValueRange", &QCPBars::getValueRange)
              .addFunction("moveAbove", &QCPBars::moveAbove)
              .addFunction("moveBelow", &QCPBars::moveBelow)
              .addFunction("stackingGap", &QCPBars::stackingGap)
              .addFunction("setContainer", static_cast<void(QCPBars::*)(QSharedPointer<QCPBarsDataContainer>)>(&QCPBars::setData))
              .addFunction("setBarsGroup", &QCPBars::setBarsGroup)
              .addFunction("setBaseValue", &QCPBars::setBaseValue)
              .addFunction("setVector", static_cast<void(QCPBars::*)(const QVector<double>&, const QVector<double>&, bool)>(&QCPBars::setData))
              .addFunction("setStackingGap", &QCPBars::setStackingGap)
              .addFunction("setWidth", &QCPBars::setWidth)
              .addFunction("setWidthType", &QCPBars::setWidthType)
              .addFunction("width", &QCPBars::width)
              .addFunction("widthType", &QCPBars::widthType)
            .endClass()

          .deriveClass<QCPAbstractPlottable1D<QCPCurveData>, QCPAbstractPlottable>("AbstractPlottable1D_QCPCurveData")
            ABSTRACT_PLOTTABLE_1D_FUNCTIONS(QCPAbstractPlottable1D<QCPCurveData>)
          .endClass()
            .deriveClass<QCPCurve, QCPAbstractPlottable1D<QCPCurveData>>("Curve")
              .addFunction("addData2", static_cast<void(QCPCurve::*)(double, double)>(&QCPCurve::addData))
              .addFunction("addData3", static_cast<void(QCPCurve::*)(double, double, double)>(&QCPCurve::addData))
              .addFunction("addVector2", static_cast<void(QCPCurve::*)(const QVector<double>&, const QVector<double>&)>(&QCPCurve::addData))
              .addFunction("addVector3", static_cast<void(QCPCurve::*)(const QVector<double>&, const QVector<double>&, const QVector<double>&, bool)>(&QCPCurve::addData))
              .addFunction("data", &QCPCurve::data)
              .addFunction("getKeyRange", &QCPCurve::getKeyRange)
              .addFunction("getValueRange", &QCPCurve::getValueRange)
              .addFunction("lineStyle", &QCPCurve::lineStyle)
              .addFunction("scatterSkip", &QCPCurve::scatterSkip)
              .addFunction("scatterStyle", &QCPCurve::scatterStyle)
              .addFunction("setContainer", static_cast<void(QCPCurve::*)(QSharedPointer<QCPCurveDataContainer>)>(&QCPCurve::setData))
              .addFunction("setLineStyle", &QCPCurve::setLineStyle)
              .addFunction("setScatterSkip", &QCPCurve::setScatterSkip)
              .addFunction("setScatterStyle", &QCPCurve::setScatterStyle)
              .addFunction("setVector2", static_cast<void(QCPCurve::*)(const QVector<double>&, const QVector<double>&)>(&QCPCurve::setData))
              .addFunction("setVector3", static_cast<void(QCPCurve::*)(const QVector<double>&, const QVector<double>&, const QVector<double>&, bool)>(&QCPCurve::setData))
            .endClass()

          .deriveClass<QCPAbstractPlottable1D<QCPFinancialData>, QCPAbstractPlottable>("AbstractPlottable1D_QCPFinancialData")
            ABSTRACT_PLOTTABLE_1D_FUNCTIONS(QCPAbstractPlottable1D<QCPFinancialData>)
          .endClass()
            .deriveClass<QCPFinancial, QCPAbstractPlottable1D<QCPFinancialData>>("Financial")
              .addStaticFunction("timeSeriesToOhlc", &QCPFinancial::timeSeriesToOhlc)

              .addFunction("addData", static_cast<void(QCPFinancial::*)(double, double, double, double, double)>(&QCPFinancial::addData))
              .addFunction("addVector", static_cast<void(QCPFinancial::*)(const QVector<double>&, const QVector<double>&, const QVector<double>&, const QVector<double>&, const QVector<double>&, bool)>(&QCPFinancial::addData))
              .addFunction("brushPositive", &QCPFinancial::brushPositive)
              .addFunction("brushNegative", &QCPFinancial::brushNegative)
              .addFunction("chartStyle", &QCPFinancial::chartStyle)
              .addFunction("data", &QCPFinancial::data)
              .addFunction("getKeyRange", &QCPFinancial::getKeyRange)
              .addFunction("getValueRange", &QCPFinancial::getValueRange)
              .addFunction("penPositive", &QCPFinancial::penPositive)
              .addFunction("penNegative", &QCPFinancial::penNegative)
              .addFunction("setBrushPositive", &QCPFinancial::setBrushPositive)
              .addFunction("setBrushNegative", &QCPFinancial::setBrushNegative)
              .addFunction("setChartStyle", &QCPFinancial::setChartStyle)
              .addFunction("setContainer", static_cast<void(QCPFinancial::*)(QSharedPointer<QCPFinancialDataContainer>)>(&QCPFinancial::setData))
              .addFunction("setPenPositive", &QCPFinancial::setPenPositive)
              .addFunction("setPenNegative", &QCPFinancial::setPenNegative)
              .addFunction("setTwoColored", &QCPFinancial::setTwoColored)
              .addFunction("setVector", static_cast<void(QCPFinancial::*)(const QVector<double>&, const QVector<double>&, const QVector<double>&, const QVector<double>&, const QVector<double>&, bool)>(&QCPFinancial::setData))
              .addFunction("setWidth", &QCPFinancial::setWidth)
              .addFunction("setWidthType", &QCPFinancial::setWidthType)
              .addFunction("twoColored", &QCPFinancial::twoColored)
              .addFunction("width", &QCPFinancial::width)
              .addFunction("widthType", &QCPFinancial::widthType)
            .endClass()

          .deriveClass<QCPAbstractPlottable1D<QCPGraphData>, QCPAbstractPlottable>("AbstractPlottable1D_QCPGraphData")
            ABSTRACT_PLOTTABLE_1D_FUNCTIONS(QCPAbstractPlottable1D<QCPGraphData>)
          .endClass()
            .deriveClass<QCPGraph, QCPAbstractPlottable1D<QCPGraphData> >("Graph")
              .addFunction("adaptiveSampling", &QCPGraph::adaptiveSampling)
              .addFunction("addData", static_cast<void(QCPGraph::*)(double, double)>(&QCPGraph::addData))
              .addFunction("addVector", static_cast<void(QCPGraph::*)(const QVector<double>&, const QVector<double>&, bool)>(&QCPGraph::addData))
              .addFunction("channelFillGraph", &QCPGraph::channelFillGraph)
              .addFunction("data", &QCPGraph::data)
              .addFunction("getKeyRange", &QCPGraph::getKeyRange)
              .addFunction("getValueRange", &QCPGraph::getValueRange)
              .addFunction("lineStyle", &QCPGraph::lineStyle)
              .addFunction("scatterSkip", &QCPGraph::scatterSkip)
              .addFunction("scatterStyle", &QCPGraph::scatterStyle)
              .addFunction("setAdaptiveSampling", &QCPGraph::setAdaptiveSampling)
              .addFunction("setChannelFillGraph", &QCPGraph::setChannelFillGraph)
              .addFunction("setContainer", static_cast<void(QCPGraph::*)(QSharedPointer<QCPGraphDataContainer>)>(&QCPGraph::setData))
              .addFunction("setLineStyle", &QCPGraph::setLineStyle)
              .addFunction("setScatterSkip", &QCPGraph::setScatterSkip)
              .addFunction("setScatterStyle", &QCPGraph::setScatterStyle)
              .addFunction("setVector", static_cast<void(QCPGraph::*)(const QVector<double>&, const QVector<double>&, bool)>(&QCPGraph::setData))
            .endClass()

          .deriveClass<QCPAbstractPlottable1D<QCPStatisticalBoxData>, QCPAbstractPlottable>("AbstractPlottable1D_QCPStatisticalBoxData")
            ABSTRACT_PLOTTABLE_1D_FUNCTIONS(QCPAbstractPlottable1D<QCPStatisticalBoxData>)
          .endClass()
            .deriveClass<QCPStatisticalBox, QCPAbstractPlottable1D<QCPStatisticalBoxData>>("StatisticalBox")
              .addFunction("addData", static_cast<void(QCPStatisticalBox::*)(double, double, double, double, double, double, const QVector<double>&)>(&QCPStatisticalBox::addData))
              .addFunction("addVector", static_cast<void(QCPStatisticalBox::*)(const QVector<double>&, const QVector<double>&, const QVector<double>&, const QVector<double>&, const QVector<double>&, const QVector<double>&, bool)>(&QCPStatisticalBox::addData))
              .addFunction("data", &QCPStatisticalBox::data)
              .addFunction("getKeyRange", &QCPStatisticalBox::getKeyRange)
              .addFunction("getValueRange", &QCPStatisticalBox::getValueRange)
              .addFunction("setBrush", &QCPStatisticalBox::setBrush)
              .addFunction("setContainer", static_cast<void(QCPStatisticalBox::*)(QSharedPointer<QCPStatisticalBoxDataContainer>)>(&QCPStatisticalBox::setData))
              .addFunction("setMedianPen", &QCPStatisticalBox::setMedianPen)
              .addFunction("setOutlierStyle", &QCPStatisticalBox::setOutlierStyle)
              .addFunction("setVector", static_cast<void(QCPStatisticalBox::*)(const QVector<double>&, const QVector<double>&, const QVector<double>&, const QVector<double>&, const QVector<double>&, const QVector<double>&, bool)>(&QCPStatisticalBox::setData))
              .addFunction("setWidth", &QCPStatisticalBox::setWidth)
              .addFunction("setWhiskerAntialiased", &QCPStatisticalBox::setWhiskerAntialiased)
              .addFunction("setWhiskerBarPen", &QCPStatisticalBox::setWhiskerBarPen)
              .addFunction("setWhiskerPen", &QCPStatisticalBox::setWhiskerPen)
              .addFunction("setWhiskerWidth", &QCPStatisticalBox::setWhiskerWidth)
            .endClass()

      .endNamespace()
    ;

#undef ABSTRACT_PLOTTABLE_1D_FUNCTIONS
}

static void QcpElement2Lua(lua_State* L, const char* ns)
{
    luabridge::getGlobalNamespace(L)
      .beginNamespace(ns)

        .deriveClass<QCPLayoutElement, QCPLayerable>("LayoutElement")
          .addFunction("elements", &QCPLayoutElement::elements)
          .addFunction("layout", &QCPLayoutElement::layout)
          .addFunction("minimumOuterSizeHint", &QCPLayoutElement::minimumOuterSizeHint)
          .addFunction("maximumOuterSizeHint", &QCPLayoutElement::maximumOuterSizeHint)
          .addFunction("outerRect", &QCPLayoutElement::outerRect)
          .addFunction("rect", &QCPLayoutElement::rect)
          .addFunction("setAutoMargins", &QCPLayoutElement::setAutoMargins)
          .addFunction("setMinimumMargins", &QCPLayoutElement::setMinimumMargins)
          .addFunction("setMinimumSizeXY", static_cast<void(QCPLayoutElement::*)(int, int)>(&QCPLayoutElement::setMinimumSize))
          .addFunction("setMaximumSizeXY", static_cast<void(QCPLayoutElement::*)(int, int)>(&QCPLayoutElement::setMaximumSize))
          .addFunction("setMargins", &QCPLayoutElement::setMargins)
          .addFunction("setMarginGroup", &QCPLayoutElement::setMarginGroup)
          .addFunction("setOuterRect", &QCPLayoutElement::setOuterRect)
          .addFunction("setSizeConstraintRect", &QCPLayoutElement::setSizeConstraintRect)
        .endClass()

          .deriveClass<QCPAbstractLegendItem, QCPLayoutElement>("AbstractLegendItem")
          .endClass()

          .deriveClass<QCPPlottableLegendItem, QCPAbstractLegendItem>("QCPPlottableLegendItem")
          .endClass()

          .deriveClass<QCPAxisRect, QCPLayoutElement>("AxisRect")
            .addFunction("addAxes", &QCPAxisRect::addAxes)
            .addFunction("addAxis", &QCPAxisRect::addAxis)
            .addFunction("axis", &QCPAxisRect::axis)
//            .addFunction("rect", &QCPAxisRect::rect)
            .addFunction("insetLayout", &QCPAxisRect::insetLayout)
            .addFunction("setBackgroundByPixmap", static_cast<void(QCPAxisRect::*)(const QPixmap&)>(&QCPAxisRect::setBackground))
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
                .addFunction("setIconSizeXY", static_cast<void(QCPLegend::*)(int, int)>(&QCPLegend::setIconSize))
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
        .addFunction("getStatisticalBoxData", getStatisticalBoxData)

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
}
