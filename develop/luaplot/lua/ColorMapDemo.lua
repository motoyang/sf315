-- ColorMapDemo.lua

--[[

A 2D color map with color scale. Color scales can be dragged and zoomed just like axes

--]]

local pa=require("qcpplot.print_any")
local qcp=require("qcpplot.qcp")
local qt=require("qcpplot.qt5")

-- fp means function of plot
function fp(plot)

  plot:axisRect(0):setupFullAxesBox(true)
  plot.xAxis:setLabel("x")
  plot.yAxis:setLabel("y")

  local colorMap = plot:createColorMap(plot.xAxis, plot.yAxis)

  local nx = 200
  local ny = 200
  colorMap:data():setSize(nx, ny)
  colorMap:data():setRange(luaplot.Range(-4, 4), luaplot.Range(-4, 4))
  local data = colorMap:data()
  local x, y = luaplot.ColorMapDataHelper.cellToCoord(data, 1, 2)

--  local x
--  local y
--  local z
  for xIndex = 0, nx-1 do
    for yIndex = 0, ny-1 do
      local data = colorMap:data()
      local x, y = luaplot.ColorMapDataHelper.cellToCoord(data, xIndex, yIndex)
      local r = 3 * math.sqrt(x*x+y*y) + 0.01
      local z = 2*x*(math.cos(r+2)/r-math.sin(r+2)/r)
      colorMap:data():setCell(xIndex, yIndex, z)
    end
  end

  local colorScale = plot:createColorScale(plot)
  plot:plotLayout():addElement(0, 1, colorScale)
  colorScale:setType(qcp.Axis.atRight)
  colorMap:setColorScale(colorScale)
  colorScale:axis():setLabel("Magnetic Field Strength")

  -- set the color gradient of the color map to one of the presets:
  colorMap:setGradient(luaplot.ColorGradient(qcp.ColorGradient.gpPolar))
  -- we could have also created a QCPColorGradient instance and added own colors to
  -- the gradient, see the documentation of QCPColorGradient for what's possible.
  -- rescale the data dimension (color) such that all data points lie in the span visualized by the color gradient:
  colorMap:rescaleDataRange()

  -- make sure the axis rect and color scale synchronize their bottom and top margins (so they line up):
  local marginGroup = plot:createMarginGroup(plot)
  plot:axisRect(0):setMarginGroup(qcp.msBottom + qcp.msTop, marginGroup)
  colorScale:setMarginGroup(qcp.msBottom + qcp.msTop, marginGroup)

  -- rescale the key (x) and value (y) axes so the whole color map is visible:
  plot:rescaleAxes()
--]]
end

function fw(w)
  local p = w:getPlot()
  fp(p)
end

qcp.startMainWindow(fw)

