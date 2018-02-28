-- BarChartDemo.lua

--[[

Three stacked bar charts with manual x axis tick labels

--]]

local pa=require("qcpplot.print_any")
local qcp=require("qcpplot.qcp")
local qt=require("qcpplot.qt5")

-- fp means function of plot
function fp(plot)

  -- set dark background gradient:
  local gradient = luaplot.QLinearGradient.fromXY(0, 0, 0, 400)
  gradient:setColorAt(0, luaplot.QColor.fromRGB(90, 90, 90, 255))
  gradient:setColorAt(0.38, luaplot.QColor.fromRGB(105, 105, 105, 255))
  gradient:setColorAt(1, luaplot.QColor.fromRGB(70, 70, 70, 255))
  plot:setBackgroundByBrush(luaplot.QBrush.fromGradient(gradient))

  -- create empty bar chart objects:
  local regen = plot:createBars(plot.xAxis, plot.yAxis)
  local nuclear = plot:createBars(plot.xAxis, plot.yAxis)
  local fossil = plot:createBars(plot.xAxis, plot.yAxis)

  regen:setAntialiased(false) -- gives more crisp, pixel aligned bar borders
  nuclear:setAntialiased(false)
  fossil:setAntialiased(false)
  regen:setStackingGap(1)
  nuclear:setStackingGap(1)
  fossil:setStackingGap(1)

  -- set names and colors:
  fossil:setName("Fossil fuels");
  fossil:setPen(luaplot.QPen.fromColor(luaplot.QColor.fromRGB(111, 9, 176, 255):lighter(170)))
  fossil:setBrush(luaplot.QBrush.fromColor(luaplot.QColor.fromRGB(111, 9, 176, 255), qt.SolidPattern))
  nuclear:setName("Nuclear")
  nuclear:setPen(luaplot.QPen.fromColor(luaplot.QColor.fromRGB(250, 170, 20, 255):lighter(150)))
  nuclear:setBrush(luaplot.QBrush.fromColor(luaplot.QColor.fromRGB(250, 170, 20, 255), qt.SolidPattern))
  regen:setName("Regenerative");
  regen:setPen(luaplot.QPen.fromColor(luaplot.QColor.fromRGB(0, 168, 140, 255):lighter(130)))
  regen:setBrush(luaplot.QBrush.fromColor(luaplot.QColor.fromRGB(0, 168, 140, 255), qt.SolidPattern))
  -- stack bars on top of each other:
  nuclear:moveAbove(fossil)
  regen:moveAbove(nuclear)

  -- prepare x axis with country labels:
  local ticks = {1, 2, 3, 4, 5, 6, 7}
  local labels = {"USA", "Japan", "Germany", "France", "UK", "Italy", "Canada"}
  local textTicker = plot:createAxisTickerText()
  textTicker:addTicksByVector(ticks, labels)

  plot.xAxis:setTicker(textTicker)
  plot.xAxis:setTickLabelRotation(60)
  plot.xAxis:setSubTicks(false)
  plot.xAxis:setTickLength(0, 4)
  plot.xAxis:setRange(0, 8)
  plot.xAxis:setBasePen(luaplot.QPen.fromColor(luaplot.QColor.fromGlobal(qt.white)))
  plot.xAxis:setTickPen(luaplot.QPen.fromColor(luaplot.QColor.fromGlobal(qt.white)))
  plot.xAxis:grid():setVisible(true)
  plot.xAxis:grid():setPen(luaplot.QPen.fromBrush(luaplot.QBrush.fromColor(luaplot.QColor.fromRGB(130, 130, 130, 255), qt.SolidPattern), 0, qt.DotLine, qt.SquareCap, qt.BevelJoin))
  plot.xAxis:setTickLabelColor(luaplot.QColor.fromGlobal(qt.white))
  plot.xAxis:setLabelColor(luaplot.QColor.fromGlobal(qt.white))

  -- prepare y axis:
  plot.yAxis:setRange(0, 12.1)
  plot.yAxis:setPadding(5)    -- a bit more space to the left border
  plot.yAxis:setLabel("Power Consumption in\nKilowatts per Capita (2007)")
  plot.yAxis:setBasePen(luaplot.QPen.fromColor(luaplot.QColor.fromGlobal(qt.white)))
  plot.yAxis:setTickPen(luaplot.QPen.fromColor(luaplot.QColor.fromGlobal(qt.white)))
  plot.yAxis:setSubTickPen(luaplot.QPen.fromColor(luaplot.QColor.fromGlobal(qt.white)))
  plot.yAxis:grid():setSubGridVisible(true);
  plot.yAxis:setTickLabelColor(luaplot.QColor.fromGlobal(qt.white))
  plot.yAxis:setLabelColor(luaplot.QColor.fromGlobal(qt.white))
  plot.yAxis:grid():setPen(luaplot.QPen.fromBrush(luaplot.QBrush.fromColor(luaplot.QColor.fromRGB(130, 130, 130, 255), qt.SolidPattern), 0, qt.DotLine, qt.SquareCap, qt.BevelJoin))
  plot.yAxis:grid():setSubGridPen(luaplot.QPen.fromBrush(luaplot.QBrush.fromColor(luaplot.QColor.fromRGB(130, 130, 130, 255), qt.SolidPattern), 0, qt.DotLine, qt.SquareCap, qt.BevelJoin))

  -- Add data:
  local fossilData = {0.86*10.5, 0.83*5.5, 0.84*5.5, 0.52*5.8, 0.89*5.2, 0.90*4.2, 0.67*11.2}
  local nuclearData = {0.08*10.5, 0.12*5.5, 0.12*5.5, 0.40*5.8, 0.09*5.2, 0.00*4.2, 0.07*11.2}
  local regenData   = {0.06*10.5, 0.05*5.5, 0.04*5.5, 0.06*5.8, 0.02*5.2, 0.07*4.2, 0.25*11.2}
  fossil:setVector(ticks, fossilData)
  nuclear:setVector(ticks, nuclearData)
  regen:setVector(ticks, regenData)
 
  -- setup legend:
  plot.legend:setVisible(true)
  plot:axisRect(0):insetLayout():setInsetAlignment(0, qt.AlignTop + qt.AlignHCenter)
  plot.legend:setBrush(luaplot.QBrush.fromColor(luaplot.QColor.fromRGB(255, 255, 255, 100), qt.SolidPattern))
  plot.legend:setBorderPen(luaplot.QPen.fromStyle(qt.NoPen))
  -- plot:font()返回的是const QFont，需要转换为QFont，否则setPointSize()不能调用
  local legendFont = luaplot.QFont.fromAnother(plot:font())
  legendFont:setPointSize(10)
  plot.legend:setFont(legendFont)

end

function fw(w)
  local p = w:getPlot()
  fp(p)
end

qcp.startMainWindow(fw)

