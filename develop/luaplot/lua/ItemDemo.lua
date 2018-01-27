-- ItemDemo.lua

--[[

Using items like text labels, arrows and a bracket. This is actually animated, see examples project

--]]

local pa=require("qcpplot.print_any")
local qcp=require("qcpplot.qcp")
local qt=require("qcpplot.qt5")


function fun4MW(mainWidget)
  print("start MainWindow from lua")
  
  local p = mainWidget:getPlot()
  fun4Plot(p)
end


local plot
local phaseTracer
function fun4Plot(p)
  plot = p
  plot:setInteractions(qcp.iRangeDrag + qcp.iRangeZoom)
  local graph = plot:addGraph(nil, nil)

local n = 500
local phase = 0
local k = 3
local x = {}
local y = {}
for i=0, n do
  x[i+1] = i/(n-1)*34 - 17
  y[i+1] = math.exp(-x[i+1]*x[i+1]/20.0)*math.sin(k*x[i+1]+phase)
end
--pa.print_any(x)
--pa.print_any(y)

graph:setData(x, y)
graph:setPen(luaplot.PenConstructor.fromColor(luaplot.ColorConstructor.fromGlobal(qt.blue)))
graph:rescaleKeyAxis()
plot.yAxis:setRange(-1.45, 1.65)
plot.xAxis:grid():setZeroLinePen(luaplot.PenConstructor.fromColor(luaplot.ColorConstructor.fromGlobal(qt.NoPen)))

local bracket = plot:createItemBracket(plot)
bracket.left:setCoords(-8, 1.1)
bracket.right:setCoords(8, 1.1);
bracket:setLength(13);

local font0 = luaplot.QFont(plot:font():family(), 10, -1, false)
local wavePacketText = plot:createItemText(plot)
wavePacketText.position:setParentAnchor(bracket.center, false)
wavePacketText.position:setCoords(0, -10); --move 10 pixels to the top from bracket center anchor
wavePacketText:setPositionAlignment(qt.AlignBottom + qt.AlignHCenter)
wavePacketText:setText("Wavepacket")
wavePacketText:setFont(font0)

phaseTracer = plot:createItemTracer(plot)
phaseTracer:setGraph(plot:graph(0))
phaseTracer:setGraphKey((math.pi*1.5-0)/3)
phaseTracer:setInterpolating(true)
phaseTracer:setStyle(qcp.ItemTracer.tsCircle)
phaseTracer:setPen(luaplot.PenConstructor.fromColor(luaplot.ColorConstructor.fromGlobal(qt.red)))
phaseTracer:setBrush(luaplot.BrushConstructor.fromColor(luaplot.ColorConstructor.fromGlobal(qt.red), qt.SolidPattern))
phaseTracer:setSize(7)

local font = luaplot.QFont(plot:font():family(), 9, -1, false)
local phaseTracerText = plot:createItemText(plot)
phaseTracerText.position:setType(qcp.ItemPosition.ptAxisRectRatio)
phaseTracerText:setPositionAlignment(qt.AlignRight + qt.AlignBottom)
phaseTracerText.position:setCoords(1.0, 0.95); -- lower right corner of axis rect
phaseTracerText:setText("Points of fixed\nphase define\nphase velocity vp")
phaseTracerText:setTextAlignment(qt.AlignLeft)
phaseTracerText:setFont(font)
phaseTracerText:setPadding(luaplot.QMargins(8, 0, 0, 0))


local phaseTracerArrow = plot:createItemCurve(plot)
phaseTracerArrow.start:setParentAnchor(phaseTracerText.left, false)
phaseTracerArrow.startDir:setParentAnchor(phaseTracerArrow.start, false)
phaseTracerArrow.startDir:setCoords(-40, 0)
phaseTracerArrow.theEnd:setParentAnchor(phaseTracer.position, false)
phaseTracerArrow.theEnd:setCoords(10, 10);
phaseTracerArrow.endDir:setParentAnchor(phaseTracerArrow.theEnd, false)
phaseTracerArrow.endDir:setCoords(30, 30)
phaseTracerArrow:setHead(luaplot.LineEnding(qcp.LineEnding.esSpikeArrow, 8, 10, false))
phaseTracerArrow:setTail(luaplot.LineEnding(qcp.LineEnding.esBar, (phaseTracerText.bottom:pixelPosition():y()-phaseTracerText.top:pixelPosition():y())*0.85, 10, false))

-- add the group velocity tracer (green circle)
local groupTracer = plot:createItemTracer(plot)
groupTracer:setGraph(plot:graph(0))
groupTracer:setGraphKey(5.5)
groupTracer:setInterpolating(true)
groupTracer:setStyle(qcp.ItemTracer.tsCircle)
groupTracer:setPen(luaplot.PenConstructor.fromColor(luaplot.ColorConstructor.fromGlobal(qt.green)))
groupTracer:setBrush(luaplot.BrushConstructor.fromColor(luaplot.ColorConstructor.fromGlobal(qt.green), qt.SolidPattern))
groupTracer:setSize(7)

local groupTracerText = plot:createItemText(plot)
groupTracerText.position:setType(qcp.ItemPosition.ptAxisRectRatio)
groupTracerText:setPositionAlignment(qt.AlignRight + qt.AlignTop)
groupTracerText.position:setCoords(1.0, 0.20)
groupTracerText:setText("Fixed positions in\nwave packet define\ngroup velocity vg")
groupTracerText:setTextAlignment(qt.AlignLeft)
groupTracerText:setFont(font)
groupTracerText:setPadding(luaplot.QMargins(8, 0, 0, 0))

local groupTracerArrow = plot:createItemCurve(plot)
groupTracerArrow.start:setParentAnchor(groupTracerText.left, false)
groupTracerArrow.startDir:setParentAnchor(groupTracerArrow.start, false)
groupTracerArrow.startDir:setCoords(-40, 0)
groupTracerArrow.theEnd:setCoords(5.5, 0.4)
groupTracerArrow.endDir:setParentAnchor(groupTracerArrow.theEnd, false)
groupTracerArrow.endDir:setCoords(0, -40)
groupTracerArrow:setHead(luaplot.LineEnding(qcp.LineEnding.esSpikeArrow, 8, 10, false))
groupTracerArrow:setTail(luaplot.LineEnding(qcp.LineEnding.esBar, (phaseTracerText.bottom:pixelPosition():y()-phaseTracerText.top:pixelPosition():y())*0.85, 10, false))


-- add dispersion arrow:
local arrow = plot:createItemCurve(plot)
arrow.start:setCoords(1, -1.1)
arrow.startDir:setCoords(-1, -1.3)
arrow.endDir:setCoords(-5, -0.3)
arrow.theEnd:setCoords(-10, -0.2)
arrow:setHead(luaplot.LineEnding(qcp.LineEnding.esSpikeArrow, 8, 10, false))


-- add the dispersion arrow label:
local dispersionText = plot:createItemText(plot)
dispersionText.position:setCoords(-6, -0.9)
dispersionText:setRotation(40)
dispersionText:setText("Dispersion with\nvp < vg")
dispersionText:setFont(font0)
end


local lastFpsKey = 0
local frameCount = 0
function f1()
  local secs = luaplot.AxisTickerDateTime.dateTimeToKey(luaplot.QDateTime.currentDateTime())

  -- update data to make phase move:
  local n = 500
  local phase = secs*5
  local k = 3
  local x={}
  local y={}
  for i=0, n-1 do
    x[i+1] = i/(n-1)*34 - 17;
    y[i+1] = math.exp(-x[i+1]*x[i+1]/20.0)*math.sin(k*x[i+1]+phase)
  end

  local g0 = plot:graph(0)
  g0:setData(x, y)
  phaseTracer:setGraphKey((8*math.pi+math.fmod(math.pi*1.5-phase, 6*math.pi))/k)
  plot:replot(qcp.CustomPlot.rpRefreshHint)

  local key = secs
  frameCount = frameCount + 1
  if (key - lastFpsKey >= 1) then
    local size = g0:data():size()
    local fps = frameCount/(key-lastFpsKey)
    lastFpsKey = key
    frameCount = 0

    local msg = string.format("%0.0f FPS, Total Data points: %d", fps, size)
    return msg
  end

end

qcp.startMainWindow(fun4MW, "f1")

qcp.startPlot(fun4Plot, "f1")


