-- AdvancedAxesDemo.lua

--[[

QCP supports multiple axes on one axis rect side and multiple axis rects per plot widget

--]]

--local ar=require("l1.array")
local pa=require("qcpplot.print_any")
local qcp=require("qcpplot.qcp")
local qt=require("qcpplot.qt5")

---[[
--local plot = luaplot.LuaPlot(nil)

local function fp(plot)

local layout = plot:plotLayout()
layout:clear()

local wideAxisRect = plot:createAxisRect(plot, true)
wideAxisRect:setupFullAxesBox(true)
local axis = wideAxisRect:axis(qcp.Axis.atRight, 0)
axis:setTickLabels(true)
wideAxisRect:addAxis(qcp.Axis.atLeft, nil):setTickLabelColor(luaplot.ColorConstructor.fromString("#6050F8"))

local subLayout = plot:createLayoutGrid()
local subRectLeft = plot:createAxisRect(plot, false)
local subRectRight = plot:createAxisRect(plot, false)
subRectLeft:addAxes(qcp.Axis.atLeft + qcp.Axis.atBottom)
subRectRight:addAxes(qcp.Axis.atRight + qcp.Axis.atBottom)
subRectRight:setMaximumSize(250, 150)
subRectRight:setMinimumSize(250, 150)

subRectLeft:axis(qcp.Axis.atLeft, 0):ticker():setTickCount(2)
subRectRight:axis(qcp.Axis.atRight, 0):ticker():setTickCount(2)
subRectRight:axis(qcp.Axis.atBottom, 0):ticker():setTickCount(2)
subRectLeft:axis(qcp.Axis.atBottom, 0):grid():setVisible(true)

---[[
local marginGroup=plot:createMarginGroup(plot)
subRectLeft:setMarginGroup(qcp.msLeft, marginGroup)
subRectRight:setMarginGroup(qcp.msRight, marginGroup)
wideAxisRect:setMarginGroup(qcp.msLeft + qcp.msRight, marginGroup)
--]]

subLayout:addElement(0, 0, subRectLeft)
subLayout:addElement(0, 1, subRectRight)

layout:addElement(0, 0, wideAxisRect)
layout:addElement(1, 0, subLayout)

dataCos={}
for i = 1, 21 do
  local d1={}
  d1.key = (i-1)/(21-1)*10-5
  d1.value = math.cos(d1.key)
  dataCos[i] = d1
end
--pa.print_any(dataCos)

dataGaussKey={}
dataGaussValue={}
for i = 1, 51 do
  dataGaussKey[i]=(i-1)/50*10-5
  dataGaussValue[i]=math.exp((0-dataGaussKey[i])*(dataGaussKey[i])*0.2)*1000
end
--pa.print_any(dataGaussValue)
--pa.print_any(dataGaussKey)

local mainGraphCos=plot:addGraph(wideAxisRect:axis(qcp.Axis.atBottom, 0), wideAxisRect:axis(qcp.Axis.atLeft, 0))
mainGraphCos:data():addVector(dataCos, true)
mainGraphCos:valueAxis():setRange(-1, 1)
mainGraphCos:rescaleKeyAxis()
local st1 = luaplot.ScatterStyleConstructor.fromShape(qcp.ScatterStyle.ssCircle, luaplot.PenConstructor.fromColor(luaplot.ColorConstructor.fromGlobal(qt.black)), luaplot.BrushConstructor.fromColor(luaplot.ColorConstructor.fromGlobal(qt.white), qt.SolidPattern), 6.0)
mainGraphCos:setScatterStyle(st1)
local p1 = luaplot.PenConstructor.fromColor(luaplot.ColorConstructor.fromRGB(120, 120, 120, 255))
p1:setWidthF(2.0)
mainGraphCos:setPen(p1)

local mainGraphGauss=plot:addGraph(wideAxisRect:axis(qcp.Axis.atBottom, 0), wideAxisRect:axis(qcp.Axis.atLeft, 1))
mainGraphGauss:addData(dataGaussKey, dataGaussValue, true)
p1 = luaplot.PenConstructor.fromColor(luaplot.ColorConstructor.fromString("#8070B8"))
p1:setWidthF(2.0)
mainGraphGauss:setPen(p1)

local b1 = luaplot.BrushConstructor.fromColor(luaplot.ColorConstructor.fromRGB(110, 170, 110, 30), qt.SolidPattern)
mainGraphGauss:setBrush(b1)
mainGraphCos:setChannelFillGraph(mainGraphGauss)
b1 = luaplot.BrushConstructor.fromColor(luaplot.ColorConstructor.fromRGB(255, 161, 0, 50), qt.SolidPattern)
mainGraphCos:setBrush(b1)
mainGraphGauss:valueAxis():setRange(0, 1000)
mainGraphGauss:rescaleKeyAxis()

dataRandomKey = {}
dataRandomValue = {0}
math.randomseed(os.time())
for i = 1, 100 do
  dataRandomKey[i] = (i-1)/100*10
  dataRandomValue[i] = math.random()-0.5+dataRandomValue[math.max(1, i-1)]
end

local subGraphRandom = plot:addGraph(subRectLeft:axis(qcp.Axis.atBottom, 0), subRectLeft:axis(qcp.Axis.atLeft, 0))
subGraphRandom:addData(dataRandomKey, dataRandomValue, true)
subGraphRandom:setLineStyle(qcp.Graph.lsImpulse)
local p1 = luaplot.PenConstructor.fromColor(luaplot.ColorConstructor.fromString("#FFA100"))
p1:setWidthF(1.5)
subGraphRandom:setPen(p1)
subGraphRandom:rescaleAxes(false)

local x3 = {1, 2, 3, 4}
local y3 = {2, 2.5, 4, 1.5}

local subBars = plot:createBars(subRectRight:axis(qcp.Axis.atBottom, 0), subRectRight:axis(qcp.Axis.atRight, 0))
subBars:setWidth(3/#x3)
subBars:setData(x3, y3)
local c1 = luaplot.ColorConstructor.fromGlobal(qt.black)
subBars:setPen(luaplot.PenConstructor.fromColor(c1))
subBars:setAntialiased(false)
subBars:setAntialiasedFill(false)
c1 = luaplot.ColorConstructor.fromString("#705BE8")
subBars:setBrush(luaplot.BrushConstructor.fromColor(c1, qt.SolidPattern))
subBars:keyAxis():setSubTicks(false)
subBars:rescaleAxes(false)

local intTicker = plot:createAxisTickerFixed()
intTicker:setTickStep(1.0)
intTicker:setScaleStrategy(qcp.AxisTickerFixed.ssMultiples)
subBars:keyAxis():setTicker(intTicker);
subBars:keyAxis():setRange(0.3, 4.7)
end

local function fw(w)
  p = w:getPlot()
  fp(p)
end

qcp.startPlot(fp)
qcp.startMainWindow(fw)

