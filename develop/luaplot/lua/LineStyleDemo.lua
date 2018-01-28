-- LineStyleDemo.lua

--[[

A demonstration of several line styles

--]]

--local ar=require("l1.array")
local pa=require("qcpplot.print_any")
local qcp=require("qcpplot.qcp")
local qt=require("qcpplot.qt5")

-- fp means function of plot
function fp(plot)

  plot.legend:setVisible(true)
  plot.legend:setFont(luaplot.FontConstructor.fromFamily("Helvetica", 9, -1, false))
  local pen = luaplot.QPen()
  local lineNames = {"lsNone", "lsLine", "lsStepLeft", "lsStepRight", "lsStepCenter", "lsImpulse"}

  for i=qcp.Graph.lsNone, qcp.Graph.lsImpulse do
    plot:addGraph(nil, nil)
    pen:setColor(luaplot.ColorConstructor.fromRGB(math.floor(0.5+math.sin(i*1+1.2)*80+80), math.floor(0.5+ math.sin(i*0.3+0)*80+80), math.floor(0.5+math.sin(i*0.3+1.5)*80+80), 255))
    plot:lastGraph():setPen(pen)

    local idx = i-qcp.Graph.lsNone
--    print("idx=", idx)
    local name = lineNames[idx+1]
--    print("name=", name)
    plot:lastGraph():setName(name)
    plot:lastGraph():setLineStyle(i)
    plot:lastGraph():setScatterStyle(luaplot.ScatterStyleConstructor.fromShapeAndSize(qcp.ScatterStyle.ssCircle, 5))
    -- generate data:
    local x, y = {}, {}
    for j=0, 15-1 do
      x[j+1] = j/15.0 * 5*3.14 + 0.01;
      y[j+1] = 7*math.sin(x[j+1])/x[j+1] - (i-qcp.Graph.lsNone)*5 + (qcp.Graph.lsImpulse)*5 + 2
    end
    plot:lastGraph():setVector(x, y)
    plot:lastGraph():rescaleAxes(true)
  end

  -- zoom out a bit:
  plot.yAxis:scaleRange(1.1, plot.yAxis:range():center())
  plot.xAxis:scaleRange(1.1, plot.xAxis:range():center())
  -- set blank axis lines:
  plot.xAxis:setTicks(false)
  plot.yAxis:setTicks(true)
  plot.xAxis:setTickLabels(false)
  plot.yAxis:setTickLabels(true)
  -- make top right axes clones of bottom left axes:
  plot:axisRect(0):setupFullAxesBox()

end

function fw(w)
  local p = w:getPlot()
  fp(p)
end

qcp.startMainWindow(fw)

