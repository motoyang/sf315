#include <lua.hpp>
#include <iostream>
#include "LuaBridge_cpp17/LuaBridge.h"
#include "qcp/qcustomplot.h"
#include "type4stack.h"
#include "mainwindow.h"
#include "qt2lua.h"

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
          .addBuilder("fromOffsetSecond", &Builder<QDateTime>::from<const QDate&, const QTime&, Qt::TimeSpec, int>)
          .addBuilder("fromAnother", &Builder<QDateTime>::from<const QDateTime&>)

          .addFunction("setTimeSpec", &QDateTime::setTimeSpec)
          .addFunction("toTime_t", &QDateTime::toTime_t)
          .addStaticFunction("currentDateTime", &QDateTime::currentDateTime)
        .endClass()

        .beginClass<QLocale>("QLocale")
          .addConstructor<void(*)()>()
            .addBuilder("fromString", &Builder<QLocale>::from<const QString&>)
            .addBuilder("fromLanguageAndCountry", &Builder<QLocale>::from<QLocale::Language, QLocale::Country>)
            .addBuilder("fromLanguageScriptAndCountry", &Builder<QLocale>::from<QLocale::Language, QLocale::Script, QLocale::Country>)
            .addBuilder("fromLocale", &Builder<QLocale>::from<const QLocale&>)
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
          .addBuilder("fromStyle", &Builder<QBrush>::from<Qt::BrushStyle>)
          .addBuilder("fromColor", &Builder<QBrush>::from<const QColor&, Qt::BrushStyle>)
          .addBuilder("fromGradient", &Builder<QBrush>::from<const QGradient&>)
          .addBuilder("fromPixmap", &Builder<QBrush>::from<const QPixmap&>)

          .addFunction("setStyle", &QBrush::setStyle)
        .endClass()

        .beginClass<QColor>("QColor")
          .addConstructor<void(*)()>()
          .addBuilder("fromGlobal", &Builder<QColor>::from<Qt::GlobalColor>)
          .addBuilder("fromRGB", &Builder<QColor>::from<int, int, int, int>)
          .addBuilder("fromString", &Builder<QColor>::from<const char*>)

          .addFunction("lighter", &QColor::lighter)
        .endClass()

        .beginClass<QFont>("QFont")
          .addConstructor<void(*)()>()
          .addBuilder("fromAnother", &Builder<QFont>::from<const QFont&>)
          .addBuilder("fromFamily", &Builder<QFont>::from<const QString&, int, int, bool>)

          .addFunction("family", &QFont::family)
          .addFunction("pointSize", &QFont::pointSize)
          .addFunction("setStyleName", &QFont::setStyleName)
          .addFunction("setPointSize", &QFont::setPointSize)
          .addFunction("setPointSizeF", &QFont::setPointSizeF)
          .addFunction("styleName", &QFont::styleName)
        .endClass()

        .beginClass<QGradient>("QGradient")
            .addFunction("setColorAt", &QGradient::setColorAt)
        .endClass()

          .deriveClass<QRadialGradient, QGradient>("QRadialGradient")
            .addConstructor<void(*)()>()
            .addBuilder("fromXYRadius", &Builder<QRadialGradient>::from<qreal, qreal, qreal>)
          .endClass()

          .deriveClass<QLinearGradient, QGradient>("QLinearGradient")
            .addConstructor<void(*)()>()
            .addBuilder("fromXY", &Builder<QLinearGradient>::from<qreal, qreal, qreal, qreal>)

            .addFunction("setColorAt", &QLinearGradient::setColorAt)
            .addFunction("setFinalStopXY", static_cast<void(QLinearGradient::*)(qreal, qreal)>(&QLinearGradient::setFinalStop))
            .addFunction("setStartXY", static_cast<void(QLinearGradient::*)(qreal, qreal)>(&QLinearGradient::setStart))
          .endClass()

        .beginClass<QMargins>("QMargins")
          .addConstructor<void(*)()>()
          .addBuilder("from", &Builder<QMargins>::from<int, int, int, int>)
        .endClass()

        .beginClass<QPainterPath>("QPainterPath")
          .addConstructor<void(*)()>()
          .addFunction("cubicToXY", static_cast<void(QPainterPath::*)(qreal, qreal, qreal, qreal, qreal, qreal)>(&QPainterPath::cubicTo))
        .endClass()

        .beginClass<QPen>("QPen")
          .addConstructor<void(*)()>()
          .addBuilder("fromColor", &Builder<QPen>::from<const QColor&>)
          .addBuilder("fromStyle", &Builder<QPen>::from<Qt::PenStyle>)
          .addBuilder("fromBrush", &Builder<QPen>::from<const QBrush&, qreal, Qt::PenStyle, Qt::PenCapStyle, Qt::PenJoinStyle>)

          .addFunction("setColor", &QPen::setColor)
          .addFunction("setStyle", &QPen::setStyle)
          .addFunction("setWidth", &QPen::setWidth)
          .addFunction("setWidthF", &QPen::setWidthF)
        .endClass()

        .beginClass<QPixmap>("QPixmap")
          .addConstructor<void(*)()>()
          .addBuilder("fromFile", &Builder<QPixmap>::from<const QString&, const char*, Qt::ImageConversionFlags>)

          .addFunction("scaledXY", static_cast<QPixmap(QPixmap::*)(int, int, Qt::AspectRatioMode, Qt::TransformationMode)const>(&QPixmap::scaled))
        .endClass()

        .beginClass<QPointF>("QPointF")
          .addConstructor<void(*)()>()
          .addBuilder("from", &Builder<QPointF>::from<double, double>)

          .addFunction("x", &QPointF::x)
          .addFunction("y", &QPointF::y)
        .endClass()

        .beginClass<QRect>("QRect")
          .addConstructor<void(*)()>()
          .addBuilder("from", &Builder<QRect>::from<int, int, int, int>)

          .addFunction("width", &QRect::width)
          .addFunction("height", &QRect::height)
        .endClass()

        .beginClass<QSize>("QSize")
          .addConstructor<void(*)(int, int)>()
          .addBuilder("from", &Builder<QSize>::from<int, int>)

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
