-- FinancialChartDemo.lua

--[[

QCP showing financial and stock data with the typical Candlestick and OHLC charts

--]]

local pa=require("qcpplot.print_any")
local qcp=require("qcpplot.qcp")
local qt=require("qcpplot.qt5")

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
  volumeAxisRect:axis(qcp.Axis.atBottom, 0):setLayerByName("axes")
  volumeAxisRect:axis(qcp.Axis.atBottom, 0):grid():setLayerByName("grid")
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
    volume:addData(startTime+3600*5.0*i, math.abs(v))
  end
  volumePos:setWidth(3600*4);
  volumePos:setPen(luaplot.PenConstructor.fromStyle(qt.NoPen))
  volumePos:setBrush(luaplot.BrushConstructor.fromColor(luaplot.ColorConstructor.fromRGB(100, 180, 110, 255), qt.SolidPattern))
  volumeNeg:setWidth(3600*4);
  volumeNeg:setPen(luaplot.PenConstructor.fromStyle(qt.NoPen))
  volumeNeg:setBrush(luaplot.BrushConstructor.fromColor(luaplot.ColorConstructor.fromRGB(180, 90, 90, 255), qt.SolidPattern))

  luaplot.QObject.connect(plot.xAxis, qt.SIGNAL("rangeChanged(QCPRange)"), volumeAxisRect:axis(qcp.Axis.atBottom, 0), qt.SLOT("setRange(QCPRange)"), qt.AutoConnection)
  luaplot.QObject.connect(volumeAxisRect:axis(qcp.Axis.atBottom, 0), qt.SIGNAL("rangeChanged(QCPRange)"), plot.xAxis, qt.SLOT("setRange(QCPRange)"), qt.AutoConnection)

  local dateTimeTicker = plot:createAxisTickerDateTime()
  dateTimeTicker:setDateTimeSpec(qt.UTC)
  dateTimeTicker:setDateTimeFormat("dd. MMMM")
  volumeAxisRect:axis(qcp.Axis.atBottom, 0):setTicker(dateTimeTicker)
  volumeAxisRect:axis(qcp.Axis.atBottom, 0):setTickLabelRotation(15)
  plot.xAxis:setBasePen(luaplot.PenConstructor.fromStyle(qt.NoPen))
  plot.xAxis:setTickLabels(false)
  plot.xAxis:setTicks(false)
  local dateTimeTicker2 = plot:createAxisTickerDateTime()
  dateTimeTicker2:setDateTimeSpec(qt.UTC)
  dateTimeTicker2:setDateTimeFormat("dd. MMMM")
  plot.xAxis:setTicker(dateTimeTicker2)
  plot:rescaleAxes()

  plot.xAxis:scaleRange(1.025, plot.xAxis:range():center())
  plot.yAxis:scaleRange(1.1, plot.yAxis:range():center());

  local group = plot:createMarginGroup(plot)
  plot:axisRect(0):setMarginGroup(qcp.msLeft + qcp.msRight, group)
  volumeAxisRect:setMarginGroup(qcp.msLeft + qcp.msRight, group)
end

local function fw(w)
  local p = w:getPlot()
  fp(p)
end

--qcp.startPlot(fp)
qcp.startMainWindow(fw)
