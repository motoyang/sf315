#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <lua.hpp>
#include "LuaBridge/LuaBridge.h"
#include "luaplot.h"
#include "expressionwidget.h"

ExpressionWidget::ExpressionWidget(QWidget *parent) : QWidget(parent)
{

    m_express = new QLineEdit(tr("enter your express"), this);
    m_plot = new LuaPlot(this);
    m_plot->axisRect()->setBackground(QBrush(Qt::lightGray));


  QGridLayout* widgetLayout = new QGridLayout(this);
  QVBoxLayout* rightLayout = new QVBoxLayout();
  QVBoxLayout* leftLayout = new QVBoxLayout();
  widgetLayout->addLayout(leftLayout, 0, 0);
  widgetLayout->addLayout(rightLayout, 0, 1);
  widgetLayout->setColumnStretch(1, 1);

  QHBoxLayout* expressLayout = new QHBoxLayout();
  expressLayout->addWidget(new QLabel(tr("Expression: "), this));
  expressLayout->addWidget(m_express, 1);
  rightLayout->addLayout(expressLayout);
  rightLayout->addWidget(m_plot, 1);
//  rightLayout->setRowStretch(1, 1);

  QGroupBox* colorGroup = new QGroupBox(tr("Color: "), this);
//  colorGroup->setFlat(true);
  QHBoxLayout* colorLayout = new QHBoxLayout(colorGroup);
  colorLayout->addWidget(new QLabel("Label3", this));
  colorLayout->addWidget(new QPushButton("PushButton3", this));

  leftLayout->addWidget(colorGroup);
/*
  QVBoxLayout* v1 = new QVBoxLayout();
  QVBoxLayout* v2 = new QVBoxLayout();

  QGridLayout*



  QVBoxLayout* v3 = new QVBoxLayout();
  QHBoxLayout* hl = new QHBoxLayout();
  glayout->addLayout(v1, 0, 0);
  glayout->addLayout(v2, 0, 1);
  glayout->setColumnStretch(1, 1);


//  v1->SetMaximumSize(200);
//  hl->addLayout(v1);
//  hl->addLayout(v2);
  v1->addWidget(group1);
  v1->addWidget(new QLabel("Label1", this));
  v1->addWidget(new QPushButton("PushButton1", this));
  v2->addWidget(new QLabel("label2", this));
  v2->addWidget(new QPushButton("PushButton2", this));
  */
}
