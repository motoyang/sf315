#ifndef LUAPLOT_H
#define LUAPLOT_H

// --

#include <lua.hpp>
#include "LuaBridge/LuaBridge.h"
#include "qcp/qcustomplot.h"

// ---

#define ICON_FOR_PLOT ":/a/png/Plot.png"

// --

class LuaPlot : public QCustomPlot
{
    Q_OBJECT

    lua_State* m_L;
    QTimer m_t;
    QString m_fn;

public:
    LuaPlot(QWidget* parent);
    ~LuaPlot();

    int setLuaState(lua_State* L);
    void setTimer(const char* funName, int msec);

    template<typename T, typename... Args>
    T* createInstance(Args... args)
    {
        return new T(std::forward<Args>(args)...);
    }

    template<typename T, typename... Args>
    QSharedPointer<T> createWithSharedPointer(Args... args) {
       return QSharedPointer<T>(createInstance<T>(std::forward<Args>(args)...));
    }

public slots:
    void timerSlot();
    void rescaleAll();
    void savePlot();
    void aboutPlot();

protected:

};

#endif // LUAPLOT_H
