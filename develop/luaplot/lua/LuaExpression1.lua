-- LuaExpression1.lua

--[[

--]]

local pa=require("qcpplot.print_any")
local qcp=require("qcpplot.qcp")
local qt=require("qcpplot.qt5")

Expression = {
  name = "name of the expression",
  expression = "fun_e1",
  xLower = -10, xUpper = 10,
  yLower = -10, yUpper = 10,
  pointsOfWidth = 800, pointsOfHeight = 600,
  splitInPoint = 3,
};
  
function Expression:new(e)
  local r = {}
  for k, v in pairs(self) do
    r[k] = v
  end
  for k, v in pairs(e) do
    r[k] = v
  end
  return r
end

function Expression:diff()
  local dx = (self.xUpper - self.xLower) / self.pointsOfWidth;
  local dy = (self.yUpper - self.yLower) / self.pointsOfHeight;

  return math.sqrt(dx*dx + dy*dy)
end

-----------------

local e0 = Expression:new {yLower=-1.5, yUpper = 1.5, name = "sin(x)=y", splitInPoint = 3, expression="f_e0"}
local diff = e0:diff()/10
function f_e0(x, y)
  local left = math.sin(x)
  local right = y

  if (math.abs(left - right) < diff) then return true end
  return false
end

qcp.startExpression(e0)

-----------------
--[[
local e1 = Expression:new {name = "sin(x^2+y^2)=cos(xy)", splitInPoint = 3, expression="f_e1"}
local diff = e1:diff()
function f_e1(x, y)
  local left = math.sin(x*x + y*y)
  local right = math.cos(x*y)

  if (math.abs(left - right) < diff) then return true end
  return false
end

qcp.startExpression(e1)

-----------------

local e2 = Expression:new {name = "|sin(x^2+2xy)|=sin(x-2y)", splitInPoint = 3, expression="f_e2"}
local diff = e2:diff()
function f_e2(x, y)
  local left = math.abs(math.sin(x*x+2*x*y))
  local right = math.sin(x-2*y)

  if (math.abs(left - right) < diff) then return true end
  return false
end

qcp.startExpression(e2)

-----------------

local e3 = Expression:new {name = "sin(sin(x)+cos(y))=cos(sin(xy)+cos(x))", splitInPoint = 3, expression="f_e3"}
local diff = e3:diff()
function f_e3(x, y)
  local left = math.sin(math.sin(x) + math.cos(y))
  local right = math.cos(math.sin(x*y)+math.cos(x))

  if (math.abs(left - right) < diff) then return true end
  return false
end

qcp.startExpression(e3)

-----------------

local e4 = Expression:new {name = "|sin(x^2-y^2)|=sin(x+y)+cos(xy)", splitInPoint = 3, expression="f_e4"}
local diff = e4:diff()
function f_e4(x, y)
  local left = math.abs(math.sin(x*x-y*y))
  local right = math.sin(x+y)+math.cos(x*y)

  if (math.abs(left - right) < diff) then return true end
  return false
end

qcp.startExpression(e4)

-----------------

local e5 = Expression:new {xLower=-100, xUpper=100, yLower=-100, yUpper=100, name = "x/sin(x) +- y/sin(y) = +- xy/sin(xy)", splitInPoint = 9, expression="f_e5"}
local diff = e5:diff()
function f_e5(x, y)
  local l1 = x/math.sin(x) + y/math.sin(y)
  local l2 = x/math.sin(x) - y/math.sin(y)
  local r1 = x*y/math.sin(x*y)
  local r2 = -(x*y/math.sin(x*y))

  if (math.abs(l1-r1) < diff) or (math.abs(l2-r2) < diff) or (math.abs(l1-r2) < diff) or (math.abs(l2-r1) < diff)
    then return true end
  return false
end

qcp.startExpression(e5)

-----------------

local e6 = Expression:new {name = "cos(cos(min(sin(x)+y,x+sin(y)))) - cos(sin(max(sin(y)+x,y+sin(x)))) > 0", splitInPoint = 9, expression="f_e6"}
local diff = e6:diff()
function f_e6(x, y)
  local l1 = math.cos(math.cos(math.min(math.sin(x)+y, x+math.sin(y))))
  local l2 = math.cos(math.sin(math.max(math.sin(y)+x, y+math.sin(x))))

  if (l1-l2 > 0)
    then return true end
  return false
end

qcp.startExpression(e6)

-----------------

local e7 = Expression:new {name = "sin((2^|y|)*x +- pi/4(y-|y|) - pi/2) = 0", splitInPoint = 9, expression="f_e7", xLower=-8, xUpper=8, yLower=-2, yUpper=6}
local diff = e7:diff()/100000
function f_e7(x, y)
  local l1 = (2^math.abs(y))*x
  local l2 = math.pi/4*(y-math.abs(y))
  local l3 = math.pi/2
  local s1=math.sin(l1+l2-l3)
  local s2=math.sin(l1-l2-l3)

  if (s1 < diff or s2 < diff)
    then return true end
  return false
end

qcp.startExpression(e7)

-----------------

local e8 = Expression:new {name = "y=3*x*log(x)-1.0/36*exp(-(36.0*x-36.0/exp(1))**4)", splitInPoint = 4, expression="f_e8", yLower=0, yUpper=1.2, xLower=-1.2, xUpper=0}
local diff = e8:diff()/10
function f_e8(x, y)
  local l1 = x
  local r1 = 3*y*math.log(y)-1.0/36*math.exp(-(36.0*y-36/math.exp(1))^4)

  if (math.abs(l1-r1) < diff)
    then return true end
  return false
end

qcp.startExpression(e8)
--]]
