#ifndef LUAPLOT_H
#define LUAPLOT_H

// --

#include <lua.hpp>
#include "LuaBridge/LuaBridge.h"
#include "qcp/qcustomplot.h"

// ---

#define ICON_FOR_PLOT ":/a/png/Plot.png"

// --

struct LuaExpression {
    QString name;
    QString luaFunctionName;
    double xLower, xUpper;
    double yLower, yUpper;
    double diff;
    int pointsOfWidth, pointsOfHeight;
};

// --

class LuaPlot : public QCustomPlot
{
    Q_OBJECT

    lua_State* m_L;
    QTimer m_t;
    QString m_fn;

public:
    LuaPlot(QWidget* parent);

    int setLuaState(lua_State* L);
    void setTimer(const char* funName, int msec);
    QCPCurve* addLuaEquation(const LuaExpression& e);
    QCPGraph* addLuaFunction(const LuaExpression& e);

    QCPAxisRect*            createAxisRect(QCustomPlot *parentPlot, bool setupDefaultAxes=true);
    QCPAxisTickerFixed*     createAxisTickerFixed();
    QCPAxisTickerDateTime*  createAxisTickerDateTime();
    QCPAxisTickerLog*       createAxisTickerLog();
    QCPAxisTickerPi*        createAxisTickerPi();
    QCPAxisTickerText*      createAxisTickerText();
    QCPAxisTickerTime*      createAxisTickerTime();
    QCPBars*                createBars(QCPAxis *keyAxis, QCPAxis *valueAxis);
    QCPCurve*               createCurve(QCPAxis *keyAxis, QCPAxis *valueAxis);
    QCPColorMap*            createColorMap(QCPAxis *keyAxis, QCPAxis *valueAxis);
    QCPColorScale*          createColorScale(QCustomPlot* parentPlot);
    QCPErrorBars*           createErrorBars(QCPAxis *keyAxis, QCPAxis *valueAxis);
    QCPFinancial*           createFinancial(QCPAxis *keyAxis, QCPAxis *valueAxis);
    QCPItemBracket*         createItemBracket(QCustomPlot* parentPlot);
    QCPItemCurve*           createItemCurve(QCustomPlot *parentPlot);
    QCPItemText*            createItemText(QCustomPlot *parentPlot);
    QCPItemTracer*          createItemTracer(QCustomPlot *parentPlot);
    QCPLayoutGrid*          createLayoutGrid();
    QCPMarginGroup*         createMarginGroup(QCustomPlot *parentPlot);
    QCPStatisticalBox*      createStatisticalBox(QCPAxis *keyAxis, QCPAxis *valueAxis);
    QCPTextElement*         createTextElement(QCustomPlot *parentPlot, const QString &text, const QFont &font);

    template<typename T>
    QSharedPointer<T> createDataContainer() {
            return QSharedPointer<T>(new T());
    }


public slots:
    void timerSlot();
    void rescaleAll();
    void savePlot();
    void aboutPlot();
};

#endif // LUAPLOT_H
