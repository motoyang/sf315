#include <lua.hpp>
#include <QDebug>
#include <iostream>
#include "LuaBridge_cpp17/LuaBridge.h"
#include "qcp/qcustomplot.h"
#include "utilities.h"
//#include "luaplot.h"
#include "mainwindow.h"
#include "type4stack.h"
#include "qt2lua.h"

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

// --

static void QtCore2Lua(lua_State* L, const char* ns)
{
    luabridge::getGlobalNamespace(L)
      .beginNamespace(ns)

        .beginClass<QDate>("QDate")
          .addConstructor<void(*)()>()
          .addBuilder("fromYMD", &Builder<QDate>::from<int, int, int>)
        .endClass()

        .beginClass<QDateTime>("QDateTime")
          .addConstructor<void(*)()>()
          .addBuilder("fromDate", &Builder<QDateTime>::from<const QDate&>)
          .addBuilder("fromDateAndTime", &Builder<QDateTime>::from<const QDate&, const QTime&, Qt::TimeSpec>)
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
          .addBuilder("fromHMSms", &Builder<QTime>::from<int, int, int, int>)
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

        .beginClass<QColor>("QColor")
          .addConstructor<void(*)()>()
          .addBuilder("from", &Builder<QColor>::from<>)
          .addBuilder("fromGlobal", &Builder<QColor>::from<Qt::GlobalColor>)
          .addBuilder("fromRGB", &Builder<QColor>::from<int, int, int, int>)
          .addBuilder("fromString", &Builder<QColor>::from<const char*>)

          .addFunction("lighter", &QColor::lighter)
        .endClass()
        .beginClass<ColorConstructor>("ColorConstructor")
          .addStaticFunction("fromString", &ColorConstructor::fromString)
          .addStaticFunction("fromRGB", &ColorConstructor::fromRGB)
          .addStaticFunction("fromGlobal", &ColorConstructor::fromGlobal)
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

        .beginClass<QMargins>("QMargins")
          .addConstructor<void(*)(int, int, int, int)>()
        .endClass()

        .beginClass<QPainterPath>("QPainterPath")
          .addConstructor<void(*)()>()
          .addFunction("cubicToXY", static_cast<void(QPainterPath::*)(qreal, qreal, qreal, qreal, qreal, qreal)>(&QPainterPath::cubicTo))
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

        .beginClass<QPixmap>("QPixmap")
          .addConstructor<void(*)()>()
          .addFunction("scaledXY", static_cast<QPixmap(QPixmap::*)(int, int, Qt::AspectRatioMode, Qt::TransformationMode)const>(&QPixmap::scaled))
        .endClass()
        .beginClass<PixmapConstructor>("PixmapConstructor")
          .addStaticFunction("fromFile", &PixmapConstructor::fromFile)
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

        .beginClass<QSize>("QSize")
          .addConstructor<void(*)(int, int)>()
          .addFunction("height", &QSize::height)
          .addFunction("width", &QSize::width)
          .addFunction("setHeight", &QSize::setHeight)
          .addFunction("setWidth", &QSize::setWidth)
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

          .deriveClass<QMainWindow, QWidget>("QMainWindow")
          .endClass()

            .deriveClass<MainWindow, QMainWindow>("MainWindow")
              .addConstructor<void (*) (QWidget* parent)>()
              .addFunction("getPlot", &MainWindow::getPlot)
            .endClass()

      .endNamespace()
    ;
}

void Qt2Lua(lua_State* L, const char* ns)
{
    QtCore2Lua(L, ns);
    QtGui2Lua(L, ns);
    QtWidget2Lua(L, ns);
}
