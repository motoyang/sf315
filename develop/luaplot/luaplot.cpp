#include "lua.hpp"
#include "LuaBridge/LuaBridge.h"
#include "ver.h"
#include "mainwindow.h"
#include "luaplot.h"

// --

LuaPlot::LuaPlot(QWidget *parent)
    : QCustomPlot(parent)
{
    setWindowIcon(QIcon(ICON_FOR_PLOT));
}

int LuaPlot::setLuaState(lua_State *L)
{
    m_L = L;

    lua_settop(L, 0);
    return 0;
}

void LuaPlot::setTimer(const char *funName, int msec)
{
    m_fn = funName;
    connect(&m_t, &QTimer::timeout, this, &LuaPlot::timerSlot);
    m_t.start(msec); // Interval 0 means to refresh as fast as possible
}

bool compareLR(double (*left)(double, double), double (*right)(double, double),
               double x, double y, double dx, double dy, double diff, int split)
{
//    double x0 = x;
    double y0 = y;
    dx /= split;
    dy /= split;
    for (int k = 0; k < split; ++k) {
        for (int h = 0; h < split; ++h) {
            double l = left(x, y);
            double r = right(x, y);
            if (qFabs(l - r) < diff) {
//            if (l == r) {
//                qDebug() << x << ", " << y;
                return true;
            }
            y += dy;
        }
        x += dx;
        y = y0;
    }
    return false;
}

bool LuaPlot::expressionCalc(const luabridge::LuaRef& f, double x, double y, double dx, double dy, int split)
{
    double y0 = y;
    dx /= split;
    dy /= split;
//    luabridge::LuaRef f = luabridge::getGlobal(m_L, express.toUtf8().constData());

    for (int i = 0; i < split; ++i) {
        for (int j = 0; j < split; ++j) {
            if (f(x, y)) {
                return true;
            }
            y += dy;
        }
        x += dx;
        y = y0;
    }

    return false;
}

QCPCurve *LuaPlot::addLuaExpression(const LuaExpression &e)
{
    luabridge::LuaRef ref = luabridge::getGlobal(m_L, e.expression.toUtf8().constData());

    double dx = (e.xUpper - e.xLower) / e.pointsOfWidth;
    double dy = (e.yUpper - e.yLower) / e.pointsOfHeight;

    QVector<double> keys, values;
    for (int i = 0; i < e.pointsOfWidth; ++i) {
        double x = e.xLower + i * dx;
        for (int j = 0; j < e.pointsOfHeight; ++j) {
            double y = e.yLower + j * dy;
            if (expressionCalc(ref, x, y, dx, dy, e.splitInPoint)) {
                keys.append(x);
                values.append(y);
            }
        }
    }

    qDebug() << "data size:" << keys.size();

    xAxis->setRange(e.xLower, e.xUpper);
    yAxis->setRange(e.yLower, e.yUpper);
    QCPCurve* curve = new QCPCurve(xAxis, yAxis);
    curve->setLineStyle(QCPCurve::lsNone);
    curve->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssSquare, 1));
    curve->addData(keys, values);

    return curve;
}

QCPCurve *LuaPlot::addExpression(double (*left)(double, double), double (*right)(double, double), const QCPRange &keyRange, const QCPRange &valueRange)
{
    QVector<double> keys, values;

//    QRect r(0, 0, 1000, 1000);
    QRect r = axisRect()->rect();
    double dx = keyRange.size() / r.width();
    double dy = valueRange.size() / r.height();
    double diff = qSqrt(dx * dx + dy * dy);

    for (int i = 0; i < r.width(); ++i) {
        double x = keyRange.lower + i * dx;
        for (int j = 0; j < r.height(); ++j) {
            double y = valueRange.lower + j * dy;
            if (compareLR(left, right, x, y, dx, dy, diff, 3)) {
                keys << x;
                values << y;
            }
        }
    }

    qDebug() << "dx= " << dx;
    qDebug() << "dy= " << dy;
    qDebug() << "diff= " << diff;
    qDebug() << "data size:" << keys.size();
//    qDebug() << keys;
//    qDebug() << values;

    xAxis->setRange(keyRange);
    yAxis->setRange(valueRange);
    QCPCurve* curve = new QCPCurve(xAxis, yAxis);
    curve->setLineStyle(QCPCurve::lsNone);
    curve->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssSquare, 1));
    curve->addData(keys, values);

    return curve;
}

QCPAxisRect *LuaPlot::createAxisRect(QCustomPlot *parentPlot, bool setupDefaultAxes)
{
    return new QCPAxisRect(parentPlot, setupDefaultAxes);
}

QCPAxisTickerFixed *LuaPlot::createAxisTickerFixed()
{
    return new QCPAxisTickerFixed();
}

QCPAxisTickerDateTime* LuaPlot::createAxisTickerDateTime()
{
    return new QCPAxisTickerDateTime();
}

QCPAxisTickerLog *LuaPlot::createAxisTickerLog()
{
    return new QCPAxisTickerLog();
}

QCPAxisTickerPi *LuaPlot::createAxisTickerPi()
{
    return new QCPAxisTickerPi();
}

QCPAxisTickerText *LuaPlot::createAxisTickerText()
{
    return new QCPAxisTickerText();
}

QCPAxisTickerTime *LuaPlot::createAxisTickerTime()
{
    return new QCPAxisTickerTime();
}

QCPBars *LuaPlot::createBars(QCPAxis *keyAxis, QCPAxis *valueAxis)
{
    return new QCPBars(keyAxis, valueAxis);
}

QCPLayoutGrid *LuaPlot::createLayoutGrid()
{
    return new QCPLayoutGrid();
}

QCPMarginGroup *LuaPlot::createMarginGroup(QCustomPlot *parentPlot)
{
    return new QCPMarginGroup(parentPlot);
}

QCPStatisticalBox *LuaPlot::createStatisticalBox(QCPAxis *keyAxis, QCPAxis *valueAxis)
{
    return new QCPStatisticalBox(keyAxis, valueAxis);
}

QCPTextElement *LuaPlot::createTextElement(QCustomPlot *parentPlot, const QString &text, const QFont &font)
{
    return new QCPTextElement(parentPlot, text, font);
}

QCPFinancial *LuaPlot::createFinancial(QCPAxis *keyAxis, QCPAxis *valueAxis)
{
    return new QCPFinancial(keyAxis, valueAxis);
}

QCPCurve *LuaPlot::createCurve(QCPAxis *keyAxis, QCPAxis *valueAxis)
{
    return new QCPCurve(keyAxis, valueAxis);
}

QCPColorMap *LuaPlot::createColorMap(QCPAxis *keyAxis, QCPAxis *valueAxis)
{
    return new QCPColorMap(keyAxis, valueAxis);
}

QCPColorScale *LuaPlot::createColorScale(QCustomPlot *parentPlot)
{
    return new QCPColorScale(parentPlot);
}

QCPErrorBars *LuaPlot::createErrorBars(QCPAxis *keyAxis, QCPAxis *valueAxis)
{
    return new QCPErrorBars(keyAxis, valueAxis);
}

QCPItemTracer *LuaPlot::createItemTracer(QCustomPlot *parentPlot)
{
    return new QCPItemTracer(parentPlot);
}

QCPItemCurve *LuaPlot::createItemCurve(QCustomPlot *parentPlot)
{
    return new QCPItemCurve(parentPlot);
}

QCPItemText *LuaPlot::createItemText(QCustomPlot *parentPlot)
{
    return new QCPItemText(parentPlot);
}

QCPItemBracket *LuaPlot::createItemBracket(QCustomPlot *parentPlot)
{
    return new QCPItemBracket(parentPlot);
}

void LuaPlot::timerSlot()
{
    if (!m_L) {
        return;
    }

    luabridge::LuaRef f = luabridge::getGlobal(m_L, m_fn.toUtf8().constData());
    const char * msg = f();
    if (!msg) {
        return;
    }

    QWidget* p = parentWidget();
    QWidget* pp = p ? p->parentWidget() : nullptr;
    if (pp) {
        MainWindow* w = (MainWindow*)pp;
        w->showOnStatusBar(msg);
    }
}

void LuaPlot::rescaleAll()
{
//    qDebug() << "rescaleAll";

    for (int i = 0; i < plottableCount(); ++i) {
        QCPAbstractPlottable* p = plottable(i);
        p->rescaleAxes(i);
    }

    replot();
}

void LuaPlot::savePlot()
{
    QString selectedFilter;
    QString name = QFileDialog::getSaveFileName(
                this,
                tr("Save plot to a file"),
                qApp->applicationFilePath(),
                tr("PDF files (*.pdf);;PNG files (*.png);;JPG files (*.jpg);;BMP files (*.bmp)"),
                &selectedFilter
                );

    QString extName = name.right(4).toLower();
    if (selectedFilter.contains("*.pdf", Qt::CaseInsensitive)) {
        if (extName != ".pdf") {
            name.append(".pdf");
        }
        savePdf(name);
    } else if (selectedFilter.contains("*.png", Qt::CaseInsensitive)) {
        if (extName != ".png") {
            name.append(".png");
        }
        savePng(name);
    } else if (selectedFilter.contains("*.jpg", Qt::CaseInsensitive)) {
        if (extName != ".jpg") {
            name.append(".jpg");
        }
        saveBmp(name);
    } else if (selectedFilter.contains("*.bmp", Qt::CaseInsensitive)) {
        if (extName != ".bmp") {
            name.append(".bmp");
        }
        saveBmp(name);
    }
}

void LuaPlot::aboutPlot()
{
//    qDebug() << Q_FUNC_INFO;
    QString ver = QString(
                tr("Version: %1.%2.%3.%4\n\n"
                   "You can get a plot through lua program.\n\n"
                   "This software based on:\n"
                   "\tlua from xxxxxx\n"
                   "\tluabridge from yyyyyy\n"
                   "\tQCustomPlot from zzzzzz\n"))
            .arg(VERSION_MAJOR)
            .arg(VERSION_MINOR)
            .arg(VERSION_PATCH)
            .arg(VERSION_BUILD)
            ;
    QMessageBox::about(
                this,
                tr("About luaplot"),
                ver);
}


static double l1(double x, double y)
{
//    return 51;
//    return y;
    return qCos(qSin(x*y)+qCos(x));
}

static double r1(double x, double y)
{
//    return x*x/13*13 + y*y/2*2;
//    return x*x + 2*x - 6;
    return qSin(qSin(x)+qCos(y)) ;
}

void LuaPlot::showEvent(QShowEvent *event)
{
//    addExpression(l1, r1, QCPRange(-10, 10), QCPRange(-10, 10));
}
