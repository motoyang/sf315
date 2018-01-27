#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class LuaPlot;
class QCPItemTracer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    LuaPlot* m_plot;
    void init(LuaPlot* customPlot);

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    LuaPlot* getPlot() const;
    void showOnStatusBar(const QString& msg);
    void initToolbar();

public slots:
    void bracketDataSlot();
};

#endif // MAINWINDOW_H
