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

class CalcGrid
{
    luabridge::LuaRef m_f;
    double m_diff;
    QRectF m_rect;

public:
    CalcGrid(const luabridge::LuaRef& f, double diff)
        : m_f(f), m_diff(diff)
    {}

    bool loop(double l, double b, double r, double t)
    {
        const double min_dx = m_diff * 1e-3;
        const double min_dy = m_diff * 1e-3;

        double x1 = l, y1 = b, x2 = r, y2 = t;
        do {
            double xc, yc, dx, dy;
            if (calc(x1, y1, x2, y2, xc, yc, dx, dy)) {
                return true;
            }

            if (dx < min_dx || dy  < min_dy) {
                return false;
            }

            x1 = qMax(xc - dx, l);
            y1 = qMax(yc - dy, b);
            x2 = qMin(xc + dx, r);
            y2 = qMin(yc + dy, t);
        } while (true);

        return false;
    }

    bool calc(double l, double b, double r, double t, double& xc, double& yc, double& dx, double& dy)
    {
        const int split = 4;
        double minV = std::numeric_limits<double>::max();

        dx = (r - l) / split;
        dy = (t - b) / split;
        for (int i = 0; i <= split; ++i) {
            double x = l + i * dx;
            for (int j = 0; j < split; ++j) {
                double y = b + j * dy;
                double v = m_f(x, y);
                if (v < m_diff) {
                    return true;
                }
                if (v < minV) {
                    minV = v;
                    xc = x;
                    yc = y;
                }
            }
        }

        return false;
    }
};

QCPCurve *LuaPlot::addLuaEquation(const LuaExpression &e)
{
    luabridge::LuaRef f = luabridge::getGlobal(m_L, e.luaFunctionName.toUtf8().constData());
    CalcGrid cg(f, e.diff);

    QVector<double> keys, values;
    double dx = (e.xUpper - e.xLower) / e.pointsOfWidth;
    double dy = (e.yUpper - e.yLower) / e.pointsOfHeight;

    double x = e.xLower;
    double y = e.yLower;
    for (int i = 0; i <= e.pointsOfWidth; ++i) {
        for (int j = 0; j <= e.pointsOfHeight; ++j) {
            if (cg.loop(x, y, x + dx, y + dy)) {
                keys.append(x);
                values.append(y);
            }
            y += dy;
        }
        x += dx;
        y = e.yLower;
    }

    QCPCurve* curve = new QCPCurve(xAxis, yAxis);
    curve->addData(keys, values);
//    curve->setLineStyle(QCPCurve::lsNone);
//    rescaleAll();

    return curve;
}

QCPGraph *LuaPlot::addLuaFunction(const LuaExpression &e)
{
    luabridge::LuaRef f = luabridge::getGlobal(m_L, e.luaFunctionName.toUtf8().constData());

    QVector<double> keys, values;
    double dx = (e.xUpper - e.xLower) / e.pointsOfWidth;

    double x = e.xLower + dx/2;
    for (int i = 0; i <= e.pointsOfWidth; ++i) {
        double y = f(x);
        keys.append(x);
        values.append(y);
        x += dx;
    }

    QCPGraph* g = addGraph();
    g->addData(keys, values);

    return g;
}

QCPCurve *LuaPlot::addLuaLogic(const LuaExpression &e)
{
    luabridge::LuaRef f = luabridge::getGlobal(m_L, e.luaFunctionName.toUtf8().constData());

    QVector<double> keys, values;
    double dx = (e.xUpper - e.xLower) / e.pointsOfWidth;
    double dy = (e.yUpper - e.yLower) / e.pointsOfHeight;

    double x = e.xLower + dx/2;
    double y = e.yLower + dy/2;
    for (int i = 0; i <= e.pointsOfWidth; ++i) {
        for (int j = 0; j <= e.pointsOfHeight; ++j) {
            if (f(x, y)) {
                keys.append(x);
                values.append(y);
            }
            y += dy;
        }
        x += dx;
        y = e.yLower;
    }

    QCPCurve* curve = new QCPCurve(xAxis, yAxis);
    curve->addData(keys, values);
//    curve->setLineStyle(QCPCurve::lsNone);

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

