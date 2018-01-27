-- SimpleDemo.lua

--[[

A simple decaying sine function with fill and its exponential envelope in red

--]]

--local ar=require("l1.array")
local pa=require("qcpplot.print_any")
local qcp=require("qcpplot.qcp")
local qt=require("qcpplot.qt5")

-- fp means function of plot
function fp(plot)

  -- add two graph into the plot
  plot:addGraph(nil, nil)
  plot:addGraph(nil, nil)
  plot:graph(0):setPen(luaplot.PenConstructor.fromColor(luaplot.ColorConstructor.fromGlobal(qt.blue)))
  plot:graph(0):setBrush(luaplot.BrushConstructor.fromColor(luaplot.ColorConstructor.fromRGB(0, 0, 255, 20), qt.SolidPattern))
  plot:graph(1):setPen(luaplot.PenConstructor.fromColor(luaplot.ColorConstructor.fromGlobal(qt.red)))

  -- get data
  local x={}
  local y0={}
  local y1={}
  for i = 0, 251 do
    x[i+1] = i
    y0[i+1] = math.exp(-i/150.0)*math.cos(i/10.0)
    y1[i+1] = math.exp(-i/150.0)
  end
--pa.print_any(x)
--pa.print_any(y0)
--pa.print_any(y1)

  -- and add these data to these graph
  plot:graph(0):setData(x, y0, true)
  plot:graph(1):setData(x, y1, true)

  -- adjust the plots
  plot.xAxis2:setVisible(true)
  plot.xAxis2:setTickLabels(false);
  plot.yAxis2:setVisible(true);
  plot.yAxis2:setTickLabels(false);

  luaplot.connect(plot.xAxis, "rangeChanged(QCPRange)", plot.xAxis2, "setRange(QCPRange)", qt.AutoConnection)
  luaplot.connect(plot.yAxis, "rangeChanged(QCPRange)", plot.yAxis2, "setRange(QCPRange)", qt.AutoConnection)

  plot:graph(0):rescaleAxes(false)
  plot:graph(1):rescaleAxes(true)

end

function fw(w)
  local p = w:getPlot()
  fp(p)
end

qcp.startMainWindow(fw)

