-- ParametricCurvesDemo.lua

--[[

Parametric curves with translucent gradient filling

--]]

local pa=require("qcpplot.print_any")
local qcp=require("qcpplot.qcp")
local qt=require("qcpplot.qt5")

local function fp(plot)
  local fermatSpiral1 = plot:createCurve(plot.xAxis, plot.yAxis)
  local fermatSpiral2 = plot:createCurve(plot.xAxis, plot.yAxis)
  local deltoidRadial = plot:createCurve(plot.xAxis, plot.yAxis)
  local pointCount = 500

  local dataSpiral1 = {}
  local dataSpiral2={}
  local dataDeltoid={}
  for i = 0, pointCount-1 do
    local phi = i/(pointCount-1)*8*math.pi
    local theta = i/(pointCount-1)*2*math.pi

    dataSpiral1[i+1] = {["t"]=i, ["key"]= math.sqrt(phi) * math.cos(phi), ["value"]=math.sqrt(phi) * math.sin(phi)}
    dataSpiral2[i+1] = {["t"]=i, ["key"]= -dataSpiral1[i+1].key, ["value"]= -dataSpiral1[i+1].value}
    dataDeltoid[i+1] = {["t"]=i, ["key"]= 2*math.cos(2*theta)+math.cos(1*theta)+2*math.sin(theta),
        ["value"]=2*math.sin(2*theta)-math.sin(1*theta)}
  end

  fermatSpiral1:data():addVector(dataSpiral1, true)
  fermatSpiral2:data():addVector(dataSpiral2, true)
  deltoidRadial:data():addVector(dataDeltoid, true)

  fermatSpiral1:setPen(luaplot.QPen.fromColor(luaplot.QColor.fromGlobal(qt.blue)))
  fermatSpiral1:setBrush(luaplot.QBrush.fromColor(luaplot.QColor.fromRGB(0, 0, 255, 20), qt.SolidPattern))
  fermatSpiral2:setPen(luaplot.QPen.fromColor(luaplot.QColor.fromRGB(255, 120, 0, 255)))
  fermatSpiral2:setBrush(luaplot.QBrush.fromColor(luaplot.QColor.fromRGB(255, 120, 0, 30), qt.SolidPattern))
  local radialGrad = luaplot.QRadialGradient.fromXYRadius(310, 180, 200)
  radialGrad:setColorAt(0, luaplot.QColor.fromRGB(170, 20, 240, 100))
  radialGrad:setColorAt(0.5, luaplot.QColor.fromRGB(20, 10, 255, 40))
  radialGrad:setColorAt(1,luaplot.QColor.fromRGB(120, 20, 240, 10))
  deltoidRadial:setPen(luaplot.QPen.fromColor(luaplot.QColor.fromRGB(170, 20, 240, 255)))
  deltoidRadial:setBrush(luaplot.QBrush.fromGradient(radialGrad))

  plot:axisRect(0):setupFullAxesBox()
  plot:rescaleAxes()

end

local function fw(w)
  local p = w:getPlot()
  fp(p)
end

qcp.startMainWindow(fw)

