-- LuaRegion.lua

--[[

--]]

local pa=require("qcpplot.print_any")
local qcp=require("qcpplot.qcp")
local qt=require("qcpplot.qt5")

--------------------

function fp1(p)
  local r = qcp.Region:new{plot=p, yLower = -1.5, yUpper = 1.5, name="函数: y=sin(x)"}

  local g1 = r:addFunction(math.sin)
  g1:setBrush(luaplot.BrushConstructor.fromColor(luaplot.ColorConstructor.fromRGB(20, 20, 20, 20), qt.SolidPattern))

  local g2 = r:addFunction(function (x) return -0.5*x - 0.5 end)
  g2:setPen(luaplot.PenConstructor.fromColor(luaplot.ColorConstructor.fromGlobal(qt.green)))

  local g3 = r:addFunction(function (x) return 1.2*math.cos(x) end)
  g3:setBrush(luaplot.BrushConstructor.fromColor(luaplot.ColorConstructor.fromRGB(40, 40, 10, 20), qt.SolidPattern))
---[[
  local curve = r:addEquation(
    function (x, y)
      local r = y
      local l1 = math.sin(x)
      local l2 = 1.2*math.cos(x)
      local v = math.max(math.abs(l1-r), math.abs(l2-r))
      return v, 0
    end
  )
  curve:setPen(luaplot.PenConstructor.fromColor(luaplot.ColorConstructor.fromGlobal(qt.red)))
  curve:setScatterStyle(luaplot.ScatterStyleConstructor.fromShapeAndSize(qcp.ScatterStyle.ssCircle, 16))
--]]
end
qcp.startPlot(fp1)

function fp2(p)
  local r = qcp.Region:new{plot=p,name="addLogic to plot",}
  r:addLogic(
    function (x, y) 
      if (math.abs(math.sqrt(x^2+y^2)-8)<0.01 or math.sqrt(x^2+y^2)<8 and x<=0 and math.sqrt(x^2+(y-4)^2)>4 or math.sqrt(x^2+(y+4)^2)<4 or math.sqrt(x^2+(y-4)^2)<1) and math.sqrt(x^2+(y+4)^2)>1
      then return true end
      return false
    end
  )
end
qcp.startPlot(fp2)

function fp3(p)
  local r = qcp.Region:new{plot=p,name="another logic",}
  r:addLogic(
    function (x, y) 
      local l1 = math.cos(math.cos(math.min(math.sin(x)+y, x+math.sin(y))))
      local l2 = math.cos(math.sin(math.max(math.sin(y)+x, y+math.sin(x))))

      if (l1-l2 > 0) then return true end
      return false
    end
  )
end

function fp4(p)
  local r = qcp.Region:new{plot=p,}
  r:addEquation(
    function (x, y) 
      local left = math.exp(math.sin(x)+math.cos(y))
      local right = math.sin(math.exp(x+y))
      return left, right
    end
  )
end
qcp.startPlot(fp4)

function fp5(p)
  local r = qcp.Region:new{plot=p, name="函数原型：x/sin(x) +- y/sin(y) = +- x*y/sin(xy)",}
  r:addEquation(
    function (x, y) 
      -- 函数原型：x/sin(x) +- y/sin(y) = +- x*y/sin(xy)
      local l1 = x/math.sin(x)
      local l2 = y/math.sin(y)
      local r = x*y/math.sin(x*y)
 
      local minV = math.abs(l1+l2 - r)
      minV = math.min(minV, math.abs(l1+l2 + r))
      minV = math.min(minV, math.abs(l1-l2 - r))
      minV = math.min(minV, math.abs(l1-l2 + r))

      return minV, 0
    end
  )
end
qcp.startPlot(fp5)

--------------------
