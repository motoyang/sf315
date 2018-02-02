#ifndef EXPRESSIONWIDGET_H
#define EXPRESSIONWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include "luaplot.h"

class ExpressionWidget : public QWidget
{
    Q_OBJECT

  QLineEdit* m_express;
  LuaPlot* m_plot;

public:
    explicit ExpressionWidget(QWidget *parent = nullptr);

signals:

public slots:
};

#endif // EXPRESSIONWIDGET_H
