-- LuaExpression2.lua

--[[

--]]

local pa=require("qcpplot.print_any")
local qcp=require("qcpplot.qcp")
local qt=require("qcpplot.qt5")

local g0 = qcp.Expression:new {name = "太极图方程"}
function g0:logic(x, y)
  if (math.abs(math.sqrt(x^2+y^2)-8)<0.01 or math.sqrt(x^2+y^2)<8 and x<=0 and math.sqrt(x^2+(y-4)^2)>4 or math.sqrt(x^2+(y+4)^2)<4 or math.sqrt(x^2+(y-4)^2)<1) and math.sqrt(x^2+(y+4)^2)>1
  then return true end
  return false
end
qcp.startLogic(g0)

local g1 = qcp.Expression:new {name = "有趣的图sss"}
function g1:logic(x, y)
  local l1 = math.cos(math.cos(math.min(math.sin(x)+y, x+math.sin(y))))
  local l2 = math.cos(math.sin(math.max(math.sin(y)+x, y+math.sin(x))))

  if (l1-l2 > 0) then return true end
  return false
end
qcp.startLogic(g1)

local g2 = qcp.Expression:new {name = "有趣的图a", xLower=-100, xUpper=100, yLower=-100, yUpper=100,}
function g2:logic(x, y)
  local left1 = (x*x+y*y)%80
  local right1 = 5

  if (left1 < right1) then return true end
  return false
end
qcp.startLogic(g2)

---[[
local f0 = qcp.Expression:new {pointsOfWidth = 100, pointsOfHeight =100, diff = 1e-10, yLower=-1.5, yUpper = 1.5, name = "正弦曲线333"}
function f0:fun(x)
  return math.cos(x)
end
qcp.startFunction(f0)

local f1 = qcp.Expression:new {xLower=0, xUpper=1.2, yLower=-1.5, yUpper=0, name = "温柔曲线333"}
function f1:fun(y)
  return 3*y*math.log(y)-1.0/36*math.exp(-(36.0*y-36/math.exp(1))^4)
end
qcp.startFunction(f1)


--[[
local e0 = qcp.Expression:new {pointsOfWidth = 0, pointsOfHeight =0, diff = 1e-10, yLower=-1.5, yUpper = 1.5, name = "正弦曲线333"}
function e0:equation(x, y)
  local left = math.sin(x)
  local right = y

  return left, right
end
qcp.startEquation(e0)
--]]

local e5 = qcp.Expression:new {name = "有趣的图5",}
function e5:equation(x, y)
  local l1 = x/math.sin(x) + y/math.sin(y)
  local l2 = x/math.sin(x) - y/math.sin(y)
  local r1 = x*y/math.sin(x*y)
  local r2 = -(x*y/math.sin(x*y))

  if (math.abs(l1-r1) < self.diff) or (math.abs(l2-r2) < self.diff) or (math.abs(l1-r2) < self.diff) or (math.abs(l2-r1) < self.diff)

  return left, right
end
qcp.startEquation(e5)

local e1 = qcp.Expression:new {name = "有趣的图1",}
function e1:equation(x, y)
  local left = math.exp(math.sin(x)+math.cos(y))
  local right = math.sin(math.exp(x+y))
  return left, right
end
qcp.startEquation(e1)

--[[
local b0 = qcp.Expression:new {name = "有趣的图", }
function b0:equation(x, y)
  local left = math.sin(x*x + y*y)
  local right = math.cos(x*y)

  return left, right
end
qcp.startEquation(b0)
--]]
