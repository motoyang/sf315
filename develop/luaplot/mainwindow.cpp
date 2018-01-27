#include <QtWidgets>
#include <lua.hpp>
#include "luaplot.h"
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowIcon(QIcon(ICON_FOR_PLOT));

    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    m_plot = new LuaPlot(central);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(m_plot);
    central->setLayout(layout);

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

void MainWindow::init(LuaPlot* customPlot)
{
    // configure axis rect:
    customPlot->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom); // this will also allow rescaling the color scale by dragging/zooming
    customPlot->axisRect()->setupFullAxesBox(true);
    customPlot->xAxis->setLabel("x");
    customPlot->yAxis->setLabel("y");

    // set up the QCPColorMap:
    QCPColorMap *colorMap = new QCPColorMap(customPlot->xAxis, customPlot->yAxis);
    int nx = 200;
    int ny = 200;
    colorMap->data()->setSize(nx, ny); // we want the color map to have nx * ny data points
    colorMap->data()->setRange(QCPRange(-4, 4), QCPRange(-4, 4)); // and span the coordinate range -4..4 in both key (x) and value (y) dimensions
    // now we assign some data, by accessing the QCPColorMapData instance of the color map:
    double x, y, z;
    for (int xIndex=0; xIndex<nx; ++xIndex)
    {
      for (int yIndex=0; yIndex<ny; ++yIndex)
      {
        colorMap->data()->cellToCoord(xIndex, yIndex, &x, &y);
        double r = 3*qSqrt(x*x+y*y)+1e-2;
        z = 2*x*(qCos(r+2)/r-qSin(r+2)/r); // the B field strength of dipole radiation (modulo physical constants)
        colorMap->data()->setCell(xIndex, yIndex, z);
      }
    }

    // add a color scale:
    QCPColorScale *colorScale = new QCPColorScale(customPlot);
    customPlot->plotLayout()->addElement(0, 1, colorScale); // add it to the right of the main axis rect
    colorScale->setType(QCPAxis::atRight); // scale shall be vertical bar with tick/axis labels right (actually atRight is already the default)
    colorMap->setColorScale(colorScale); // associate the color map with the color scale
    colorScale->axis()->setLabel("Magnetic Field Strength");

    // set the color gradient of the color map to one of the presets:
    colorMap->setGradient(QCPColorGradient::gpPolar);
    // we could have also created a QCPColorGradient instance and added own colors to
    // the gradient, see the documentation of QCPColorGradient for what's possible.

    // rescale the data dimension (color) such that all data points lie in the span visualized by the color gradient:
    colorMap->rescaleDataRange();

    // make sure the axis rect and color scale synchronize their bottom and top margins (so they line up):
    QCPMarginGroup *marginGroup = new QCPMarginGroup(customPlot);
    customPlot->axisRect()->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
    colorScale->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);

    // rescale the key (x) and value (y) axes so the whole color map is visible:
    customPlot->rescaleAxes();
}

void MainWindow::bracketDataSlot()
{
//    m_plot->doLua("f1");
/*
    double secs = QCPAxisTickerDateTime::dateTimeToKey(QDateTime::currentDateTime());

    // update data to make phase move:
    int n = 500;
    double phase = secs*5;
    double k = 3;
    QVector<double> x(n), y(n);
    for (int i=0; i<n; ++i)
    {
      x[i] = i/(double)(n-1)*34 - 17;
      y[i] = qExp(-x[i]*x[i]/20.0)*qSin(k*x[i]+phase);
    }
//    m_plot->graph()->setData(x, y);

//    itemDemoPhaseTracer->setGraphKey((8*M_PI+fmod(M_PI*1.5-phase, 6*M_PI))/k);

//    m_plot->replot();

    // calculate frames per second:
    double key = secs;
    static double lastFpsKey;
    static int frameCount;
    ++frameCount;
    if (key-lastFpsKey > 1) // average fps over 2 seconds
    {
      statusBar()->showMessage(
            QString("%1 FPS, Total Data points: %2")
            .arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
            .arg(m_plot->graph(0)->data()->size())
            , 0);
      lastFpsKey = key;
      frameCount = 0;
    }
*/
}
