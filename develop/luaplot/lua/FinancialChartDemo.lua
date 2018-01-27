-- FinancialChartDemo.lua

--[[

QCP showing financial and stock data with the typical Candlestick and OHLC charts

--]]

--local ar=require("l1.array")
local pa=require("qcpplot.print_any")
local qcp=require("qcpplot.qcp")
local qt=require("qcpplot.qt5")

--local plot = luaplot.LuaPlot(nil)
--local plot=luaplot.LuaPlot.createPlot(nil)
--pa.print_any(plot)

local function fp(plot)

plot.legend:setVisible(true)

local n = 500
local time={}
local value1={}
local value2={}

local start = luaplot.QDateTime(luaplot.QDate(2018,1,18))
start:setTimeSpec(qt.UTC)
local startTime = start:toTime_t()
local binSize = 3600*24
time[1]=startTime
value1[1]=60
value2[1]=20
math.randomseed(os.time())

for i =2, n do 
  time[i] = startTime + 3600 * i
  value1[i] = value1[i-1] + (math.random() - 0.5) * 10
  value2[i] = value2[i-1] + (math.random() - 0.5) * 3
end

--pa.print_any(time)
--pa.print_any(value1)
--pa.print_any(value2)

local candlesticks = plot:createFinancial(plot.xAxis, plot.yAxis)
candlesticks:setName("candlesticks")
candlesticks:setChartStyle(qcp.Financial.csCandlestick)
candlesticks:data():addContainer(luaplot.Financial.timeSeriesToOhlc(time, value1, binSize, startTime))
candlesticks:setWidth(binSize*0.9)
candlesticks:setTwoColored(true);
candlesticks:setBrushPositive(luaplot.BrushConstructor.fromColor(luaplot.ColorConstructor.fromRGB(245, 245, 245, 255), qt.SolidPattern))
candlesticks:setBrushNegative(luaplot.BrushConstructor.fromColor(luaplot.ColorConstructor.fromRGB(40, 40, 40, 255), qt.SolidPattern))
candlesticks:setPenPositive(luaplot.PenConstructor.fromColor(luaplot.ColorConstructor.fromRGB(0, 0, 0, 255)))
candlesticks:setPenNegative(luaplot.PenConstructor.fromColor(luaplot.ColorConstructor.fromRGB(0, 0, 0, 255)))

local ohlc = plot:createFinancial(plot.xAxis, plot.yAxis)
ohlc:setName("OHLC")
ohlc:setChartStyle(qcp.Financial.csOhlc)
ohlc:data():addContainer(luaplot.Financial.timeSeriesToOhlc(time, value2, binSize/3.0, startTime))
ohlc:setWidth(binSize*0.2)
ohlc:setTwoColored(true)

local volumeAxisRect = plot:createAxisRect(plot, true)
plot:plotLayout():addElement(1, 0, volumeAxisRect)
volumeAxisRect:setMaximumSize(qt.QWIDGETSIZE_MAX, 100)
volumeAxisRect:axis(qcp.Axis.atBottom, 0):setLayer("axes")
volumeAxisRect:axis(qcp.Axis.atBottom, 0):grid():setLayer("grid")
plot:plotLayout():setRowSpacing(0)

volumeAxisRect:setAutoMargins(qcp.msLeft + qcp.msRight + qcp.msBottom)
volumeAxisRect:setMargins(luaplot.QMargins(0, 0, 0, 0))
plot:setAutoAddPlottableToLegend(false)

local volumePos = plot:createBars(volumeAxisRect:axis(qcp.Axis.atBottom, 0), volumeAxisRect:axis(qcp.Axis.atLeft, 0))
local volumeNeg = plot:createBars(volumeAxisRect:axis(qcp.Axis.atBottom, 0), volumeAxisRect:axis(qcp.Axis.atLeft, 0))
for i = 0, n/5 do
  local v = math.random(0, 20000)*3 - 30000
  local volume
  if (v < 0) then volume = volumeNeg else volume = volumePos end
--  local volume = ((v < 0) or volumePos) and volumeNeg
  volume:addData(startTime+3600*5.0*i, math.abs(v))
end
volumePos:setWidth(3600*4);
volumePos:setPen(luaplot.PenConstructor.fromStyle(qt.NoPen))
volumePos:setBrush(luaplot.BrushConstructor.fromColor(luaplot.ColorConstructor.fromRGB(100, 180, 110, 255), qt.SolidPattern))
volumeNeg:setWidth(3600*4);
volumeNeg:setPen(luaplot.PenConstructor.fromStyle(qt.NoPen))
volumeNeg:setBrush(luaplot.BrushConstructor.fromColor(luaplot.ColorConstructor.fromRGB(180, 90, 90, 255), qt.SolidPattern))

luaplot.connect(plot.xAxis, "rangeChanged(QCPRange)", volumeAxisRect:axis(qcp.Axis.atBottom, 0), "setRange(QCPRange)", qt.AutoConnection)
luaplot.connect(volumeAxisRect:axis(qcp.Axis.atBottom, 0), "rangeChanged(QCPRange)", plot.xAxis, "setRange(QCPRange)", qt.AutoConnection)

local dateTimeTicker = plot:createAxisTkckerDateTime()
dateTimeTicker:setDateTimeSpec(qt.UTC)
dateTimeTicker:setDateTimeFormat("dd. MMMM")
volumeAxisRect:axis(qcp.Axis.atBottom, 0):setTicker(dateTimeTicker)
volumeAxisRect:axis(qcp.Axis.atBottom, 0):setTickLabelRotation(15)
plot.xAxis:setBasePen(luaplot.PenConstructor.fromStyle(qt.NoPen))
plot.xAxis:setTickLabels(false)
plot.xAxis:setTicks(false)
local dateTimeTicker2 = plot:createAxisTkckerDateTime()
dateTimeTicker2:setDateTimeSpec(qt.UTC)
dateTimeTicker2:setDateTimeFormat("dd. MMMM")
plot.xAxis:setTicker(dateTimeTicker2)
--plot.xAxis:setTicker(dateTimeTicker)
--plot.xAxis:setTicker(volumeAxisRect:axis(qcp.Axis.atBottom, 0):ticker())
plot:rescaleAxes()

plot.xAxis:scaleRange(1.025, plot.xAxis:range():center())
plot.yAxis:scaleRange(1.1, plot.yAxis:range():center());

local group = plot:createMarginGroup(plot)
plot:axisRect(0):setMarginGroup(qcp.msLeft + qcp.msRight, group)
volumeAxisRect:setMarginGroup(qcp.msLeft + qcp.msRight, group)

--plot:setInteractions(qcp.iRangeDrag + qcp.iRangeZoom + qcp.iSelectPlottables)
--customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

end


local function fw(w)
  local p = w:getPlot()
  fp(p)
end

qcp.startPlot(fp)
qcp.startMainWindow(fw)

--[[
plot:setWindowTitle("中华人民and Sinbad")
plot:resize(800, 600)
--plot:setAttribute(qt.WA_DeleteOnClose, true)
plot:show()
luaplot.App.exec()
--]]