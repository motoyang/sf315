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
//    init(m_plot);
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
    customPlot->addGraph(); // blue line
    customPlot->graph(0)->setPen(QPen(QColor(40, 110, 255)));
    customPlot->addGraph(); // red line
    customPlot->graph(1)->setPen(QPen(QColor(255, 110, 40)));

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s");
    customPlot->xAxis->setTicker(timeTicker);
    customPlot->axisRect()->setupFullAxesBox();
    customPlot->yAxis->setRange(-1.2, 1.2);

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->yAxis2, SLOT(setRange(QCPRange)));

    // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
//    connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
//    dataTimer.start(0); // Interval 0 means to refresh as fast as possible
}

void MainWindow::bracketDataSlot()
{
    /*
    static QTime time(QTime::currentTime());
    // calculate two new data points:
    double key = time.elapsed()/1000.0; // time elapsed since start of demo, in seconds
    static double lastPointKey = 0;
    if (key-lastPointKey > 0.002) // at most add point every 2 ms
    {
      // add data to lines:
      ui->customPlot->graph(0)->addData(key, qSin(key)+qrand()/(double)RAND_MAX*1*qSin(key/0.3843));
      ui->customPlot->graph(1)->addData(key, qCos(key)+qrand()/(double)RAND_MAX*0.5*qSin(key/0.4364));
      // rescale value (vertical) axis to fit the current data:
      //ui->customPlot->graph(0)->rescaleValueAxis();
      //ui->customPlot->graph(1)->rescaleValueAxis(true);
      lastPointKey = key;
    }
    // make key axis range scroll with the data (at a constant range size of 8):
    ui->customPlot->xAxis->setRange(key, 8, Qt::AlignRight);
    ui->customPlot->replot();

    // calculate frames per second:
    static double lastFpsKey;
    static int frameCount;
    ++frameCount;
    if (key-lastFpsKey > 2) // average fps over 2 seconds
    {
      ui->statusBar->showMessage(
            QString("%1 FPS, Total Data points: %2")
            .arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
            .arg(ui->customPlot->graph(0)->data()->size()+ui->customPlot->graph(1)->data()->size())
            , 0);
      lastFpsKey = key;
      frameCount = 0;
      */
}
