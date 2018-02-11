-- LuaExpression2.lua

--[[

--]]

local pa=require("qcpplot.print_any")
local qcp=require("qcpplot.qcp")
local qt=require("qcpplot.qt5")

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

local e1 = qcp.Expression:new {pointsOfWidth = 0, pointsOfHeight =0, diff = 1e-10, name = "有趣的图1",}
function e1:equation(x, y)
  local left = math.exp(math.sin(x)+math.cos(y))
  local right = math.sin(math.exp(x+y))
  return left, right
end
qcp.startEquation(e1)

local b0 = qcp.Expression:new {pointsOfWidth = 0, pointsOfHeight =0, diff = 1e-10, name = "有趣的图", }
function b0:equation(x, y)
  local left = math.sin(x*x + y*y)
  local right = math.cos(x*y)

  return left, right
end
qcp.startEquation(b0)
--]]
