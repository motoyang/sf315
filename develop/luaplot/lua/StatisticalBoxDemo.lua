-- StatisticalBoxDemo.lua

--[[

Statistical 5-parameter-box-plot with outliers

--]]

--local ar=require("l1.array")
local pa=require("qcpplot.print_any")
local qcp=require("qcpplot.qcp")
local qt=require("qcpplot.qt5")

-- fp means function of plot
function fp(plot)

  local statistical = plot:createStatisticalBox(plot.xAxis, plot.yAxis)
  local boxBrush = luaplot.BrushConstructor.fromColor(luaplot.ColorConstructor.fromRGB(60, 60, 255, 100), qt.SolidPattern)
  boxBrush:setStyle(qt.Dense6Pattern)
  statistical:setBrush(boxBrush)

  -- specify data:
  statistical:addData(1, 1.1, 1.9, 2.25, 2.7, 4.2, {})
  -- provide some outliers as QVector
  statistical:addData(2, 0.8, 1.6, 2.2, 3.2, 4.9, {0.7, 0.34, 0.45, 6.2, 5.84})
  statistical:addData(3, 0.2, 0.7, 1.1, 1.6, 2.9, {})

  -- prepare manual x axis labels:
  plot.xAxis:setSubTicks(false)
  plot.xAxis:setTickLength(0, 4)
  plot.xAxis:setTickLabelRotation(20)
  local textTicker = plot:createAxisTickerText()
  textTicker:addTick(1, "Sample 1");
  textTicker:addTick(2, "Sample 2");
  textTicker:addTick(3, "Control Group");
  plot.xAxis:setTicker(textTicker)

  -- prepare axes:
  plot.yAxis:setLabel("O₂ Absorption [mg]")
  plot:rescaleAxes()
  plot.xAxis:scaleRange(1.7, plot.xAxis:range():center())
  plot.yAxis:setRange(0, 7)
end

function fw(w)
  local p = w:getPlot()
  fp(p)
end

qcp.startMainWindow(fw)

