#ifndef LUAPLOT_H
#define LUAPLOT_H

#include <lua.hpp>
#include "qcp/qcustomplot.h"

// ---

#define ICON_FOR_PLOT ":/a/png/Plot.png"

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

    QCPAxisRect*            createAxisRect(QCustomPlot *parentPlot, bool setupDefaultAxes=true);
    QCPAxisTickerFixed*     createAxisTickerFixed();
    QCPAxisTickerDateTime*  createAxisTickerDateTime();
    QCPAxisTickerLog*       createAxisTickerLog();
    QCPAxisTickerText*      createAxisTickerText();
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
