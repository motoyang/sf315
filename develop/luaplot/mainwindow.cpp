#include <QtWidgets>
#include <lua.hpp>
#include "LuaBridge_cpp17/LuaBridge.h"
#include "luaplot.h"
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowIcon(QIcon(ICON_FOR_PLOT));

    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    m_plot = new LuaPlot(central);

    QHBoxLayout *layout = new QHBoxLayout(central);
    layout->addWidget(m_plot);

    resize(800, 600);
    initToolbar();
    statusBar();
}

MainWindow::~MainWindow()
{
}

LuaPlot *MainWindow::getPlot() const
{
    return m_plot;
}

void MainWindow::showOnStatusBar(const QString &msg)
{
    statusBar()->showMessage(msg);
}

void MainWindow::initToolbar()
{
    struct ActionDesc {
        QString iconName;
        QString name;
        QKeySequence key;
        QString tip;
        void (LuaPlot::*f)();
    };

    QVector<ActionDesc> actVector = {
        ActionDesc {":/a/png/arrow494.png", tr("&Home"), Qt::Key_Home, tr("Return to original size"), &LuaPlot::rescaleAll}
        , ActionDesc {":/a/png/floppy16.png", tr("&Save"), Qt::Key_Save, tr("Save the plot to a file"), &LuaPlot::savePlot}
        , ActionDesc {":/a/png/speech65.png", tr("&About"), Qt::Key_unknown, tr("About luaplot"), &LuaPlot::aboutPlot},
    };

    LuaPlot* plot = getPlot();
    QToolBar *toolBar = addToolBar(tr("&MainWindow"));

    for (int i = 0; i < actVector.size(); ++i) {
        ActionDesc ad = actVector.at(i);
        QAction* act = new QAction(QIcon(ad.iconName), ad.name, this);
        act->setShortcut(ad.key);
        act->setStatusTip(ad.tip);
        connect(act, &QAction::triggered, plot, ad.f);

        toolBar->addAction(act);
        if (i == 0 || i == 1 || i == 2) {
            toolBar->addSeparator();
        }
    }
}
