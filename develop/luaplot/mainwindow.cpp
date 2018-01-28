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
    customPlot->legend->setVisible(true);
    customPlot->legend->setFont(QFont("Helvetica",9));
    // set locale to english, so we get english decimal separator:
    customPlot->setLocale(QLocale(QLocale::English, QLocale::UnitedKingdom));
    // add confidence band graphs:
    customPlot->addGraph();
    QPen pen;
    pen.setStyle(Qt::DotLine);
    pen.setWidth(1);
    pen.setColor(QColor(180,180,180));
    customPlot->graph(0)->setName("Confidence Band 68%");
    customPlot->graph(0)->setPen(pen);
    customPlot->graph(0)->setBrush(QBrush(QColor(255,50,30,20)));
    customPlot->addGraph();
    customPlot->legend->removeItem(customPlot->legend->itemCount()-1); // don't show two confidence band graphs in legend
    customPlot->graph(1)->setPen(pen);
    customPlot->graph(0)->setChannelFillGraph(customPlot->graph(1));
    // add theory curve graph:
    customPlot->addGraph();
    pen.setStyle(Qt::DashLine);
    pen.setWidth(2);
    pen.setColor(Qt::red);
    customPlot->graph(2)->setPen(pen);
    customPlot->graph(2)->setName("Theory Curve");
    // add data point graph:
    customPlot->addGraph();
    customPlot->graph(3)->setPen(QPen(Qt::blue));
    customPlot->graph(3)->setLineStyle(QCPGraph::lsNone);
    customPlot->graph(3)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCross, 4));
    // add error bars:
    QCPErrorBars *errorBars = new QCPErrorBars(customPlot->xAxis, customPlot->yAxis);
    errorBars->removeFromLegend();
    errorBars->setAntialiased(false);
    errorBars->setDataPlottable(customPlot->graph(3));
    errorBars->setPen(QPen(QColor(180,180,180)));
    customPlot->graph(3)->setName("Measurement");

    // generate ideal sinc curve data and some randomly perturbed data for scatter plot:
    QVector<double> x0(250), y0(250);
    QVector<double> yConfUpper(250), yConfLower(250);
    for (int i=0; i<250; ++i)
    {
      x0[i] = (i/249.0-0.5)*30+0.01; // by adding a small offset we make sure not do divide by zero in next code line
      y0[i] = qSin(x0[i])/x0[i]; // sinc function
      yConfUpper[i] = y0[i]+0.15;
      yConfLower[i] = y0[i]-0.15;
      x0[i] *= 1000;
    }
    QVector<double> x1(50), y1(50), y1err(50);
    for (int i=0; i<50; ++i)
    {
      // generate a gaussian distributed random number:
      double tmp1 = rand()/(double)RAND_MAX;
      double tmp2 = rand()/(double)RAND_MAX;
      double r = qSqrt(-2*qLn(tmp1))*qCos(2*M_PI*tmp2); // box-muller transform for gaussian distribution
      // set y1 to value of y0 plus a random gaussian pertubation:
      x1[i] = (i/50.0-0.5)*30+0.25;
      y1[i] = qSin(x1[i])/x1[i]+r*0.15;
      x1[i] *= 1000;
      y1err[i] = 0.15;
    }
    // pass data to graphs and let QCustomPlot determine the axes ranges so the whole thing is visible:
    customPlot->graph(0)->setData(x0, yConfUpper);
    customPlot->graph(1)->setData(x0, yConfLower);
    customPlot->graph(2)->setData(x0, y0);
    customPlot->graph(3)->setData(x1, y1);
    errorBars->setData(y1err);
    customPlot->graph(2)->rescaleAxes();
    customPlot->graph(3)->rescaleAxes(true);
    // setup look of bottom tick labels:
    customPlot->xAxis->setTickLabelRotation(30);
    customPlot->xAxis->ticker()->setTickCount(9);
    customPlot->xAxis->setNumberFormat("ebc");
    customPlot->xAxis->setNumberPrecision(1);
    customPlot->xAxis->moveRange(-10);
    // make top right axes clones of bottom left axes. Looks prettier:
    customPlot->axisRect()->setupFullAxesBox();
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
