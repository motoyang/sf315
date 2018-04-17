-- SimplePlot.lua

--[[


--]]

local pa=require("qcpplot.print_any")
local qcp=require("qcpplot.qcp")
local qt=require("qcpplot.qt5")
local ar=require("qcpplot.array")

-- fp means function of plot
function fp(plot)

  local xx = ar.arange(0, 2.0, 0.01)
  local yy = ar.transform(xx, function(x) return 1 + math.sin(2 * math.pi * x) end)
  
  -- add two graph into the plot
  plot:addGraph(nil, nil)
  plot:graph(0):setPen(luaplot.QPen.fromColor(luaplot.QColor.fromGlobal(qt.blue)))

  -- and add these data to these graph
  plot:graph(0):setVector(xx, yy, true)

  plot.xAxis:setRange(-0.05, 2.05)
  plot.yAxis:setRange(-0.05, 2.05)
  
  -- adjust the plots
  plot.xAxis2:setVisible(true)
  plot.xAxis2:setTickLabels(false);
  plot.xAxis2:setTicks(false)
  plot.yAxis2:setVisible(true);
  plot.yAxis2:setTickLabels(false);
  plot.yAxis2:setTicks(false)
  plot.xAxis:setLabel("time (s)")
  plot.yAxis:setLabel("Voltage (mV)")
  plot.xAxis:setSubTicks(false)
  plot.yAxis:setSubTicks(false)
  plot.xAxis2:setSubTicks(false)
  plot.yAxis2:setSubTicks(false)
  
  local fixedTicker = plot:createAxisTickerFixed()
  fixedTicker:setTickStep(0.25)
  plot.xAxis:setTicker(fixedTicker)

  fixedTicker = plot:createAxisTickerFixed()
  fixedTicker:setTickStep(0.25)
  plot.yAxis:setTicker(fixedTicker)
  
--[[  
  luaplot.QObject.connect(plot.xAxis, qt.SIGNAL("rangeChanged(QCPRange)"), plot.xAxis2, qt.SLOT("setRange(QCPRange)"), qt.AutoConnection)
  luaplot.QObject.connect(plot.yAxis, qt.SIGNAL("rangeChanged(QCPRange)"), plot.yAxis2, qt.SLOT("setRange(QCPRange)"), qt.AutoConnection)
--]]
--  plot:graph(0):rescaleAxes(false)
--  plot.xAxis:grid():setSubGridVisible(true)
--  plot.yAxis:grid():setSubGridVisible(true)
  
end

function fw(w)
  local p = w:getPlot()
  fp(p)
end

qcp.startMainWindow(fw)

