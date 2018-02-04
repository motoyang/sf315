-- LuaExpression1.lua

--[[

--]]

local pa=require("qcpplot.print_any")
local qcp=require("qcpplot.qcp")
local qt=require("qcpplot.qt5")

-----------------

local a0 = qcp.Expression:new {yLower=-1.5, yUpper = 1.5, name="正弦曲线", expression = "y=sin(x)", splitInPoint = 3}
function a0:f(x, y)
  if (self.diff == nil) then
    self.diff = self:calcDefaultDiff() / 10
  end

  local left1 = math.sin(x)
  local right1 = y
  local l2 = -x/4+0.51
  local r2 = y
  local l3 = math.cos(x)
  local r3 = y

  if (math.abs(left1 - right1) < self.diff) 
    or (math.abs(l2-r2) < self.diff) 
    or (math.abs(l3-r3) < self.diff)
  then return true end
  return false
end

qcp.startExpression(a0)

-----------------

local a1 = qcp.Expression:new {yLower=-1.5, yUpper = 1.5, name="余弦曲线", expression = "y=cos(x)"}
function a1:f(x, y)
  if (self.diff == nil) then
    self.diff = self:calcDefaultDiff() / 10
  end

  local left1 = math.cos(x)
  local right1 = y

  if (math.abs(left1 - right1) < self.diff) 
  then return true end

  return false
end

local a2 = qcp.Expression:new {yLower=-3.5, yUpper = 3.5, name="k线", expression = "y=cos(x)", splitInPoint = 3}
function a2:f(x, y)
  if (self.diff == nil) then
    self.diff = self:calcDefaultDiff() / 10
  end

  local left1 = -x/8-1.1
  local right1 = y

  if (math.abs(left1 - right1) < self.diff) 
  then return true end

  return false
end

local a3 = qcp.Expression:new {yLower=-2.5, yUpper = 2.5, name="k线", expression = "y=cos(x)", splitInPoint = 6}
function a3:f(x, y)
  if (self.diff == nil) then
    self.diff = self:calcDefaultDiff() / 10
  end

  local left1 = math.cos(x)
  local right1 = -x/8-1.1
  local l2 = math.cos(x)
  local r2 = y

  if (math.abs(left1 - right1) < self.diff) and (math.abs(l2 - r2) < self.diff)
  then return true end

  return false
end

function fp(p)
  local c1 = qcp.addExpression(p, a1)
  c1:setPen(luaplot.PenConstructor.fromColor(luaplot.ColorConstructor.fromGlobal(qt.green)))
  print("c1 size = ", c1:data():size())

  local c2 = qcp.addExpression(p, a2)
  c2:setPen(luaplot.PenConstructor.fromColor(luaplot.ColorConstructor.fromGlobal(qt.red)))
  print("c2 size = ", c2:data():size())

  local c3 = qcp.addExpression(p, a3)
  c3:setPen(luaplot.PenConstructor.fromColor(luaplot.ColorConstructor.fromGlobal(qt.blue)))
  c3:setScatterStyle(luaplot.ScatterStyleConstructor.fromShapeAndSize(qcp.ScatterStyle.ssCircle, 16))
  print("c3 size = ", c3:data():size())
end

qcp.startPlot(fp)

-----------------

local e0 = qcp.Expression:new {yLower=-1.5, yUpper = 1.5, name = "正弦曲线", expression="sin(x)=y"}
function e0:f(x, y)
  if (self.diff == nil) then
    self.diff = self:calcDefaultDiff() / 10
  end

  local left = math.sin(x)
  local right = y

  if (math.abs(left - right) < self.diff) then return true end
  return false
end

qcp.startExpression(e0)

-----------------

local e1 = qcp.Expression:new {name = "有趣的图1", splitInPoint = 5, expression="sin(x^2+y^2)=cos(xy)"}
function e1:f(x, y)
  if (self.diff == nil) then
    self.diff = self:calcDefaultDiff() / 1
  end

  local left = math.sin(x*x + y*y)
  local right = math.cos(x*y)

  if (math.abs(left - right) < self.diff) then return true end
  return false
end

qcp.startExpression(e1)

-----------------

local e2 = qcp.Expression:new {name = "有趣的图2", splitInPoint = 6, expression="|sin(x^2+2xy)|=sin(x-2y)"}
function e2:f(x, y)
  if (self.diff == nil) then
    self.diff = self:calcDefaultDiff() / 1
  end

  local left = math.abs(math.sin(x*x+2*x*y))
  local right = math.sin(x-2*y)

  if (math.abs(left - right) < self.diff) then return true end
  return false
end

qcp.startExpression(e2)

-----------------

local e3 = qcp.Expression:new {name = "有趣的图3", splitInPoint = 7, expression="sin(sin(x)+cos(y))=cos(sin(xy)+cos(x))"}
function e3:f(x, y)
  if (self.diff == nil) then
    self.diff = self:calcDefaultDiff() / 1
  end

  local left = math.sin(math.sin(x) + math.cos(y))
  local right = math.cos(math.sin(x*y)+math.cos(x))

  if (math.abs(left - right) < self.diff) then return true end
  return false
end

qcp.startExpression(e3)

-----------------

local e4 = qcp.Expression:new {name = "有趣的图4", splitInPoint = 5, expression="|sin(x^2-y^2)|=sin(x+y)+cos(xy)"}
function e4:f(x, y)
  if (self.diff == nil) then
    self.diff = self:calcDefaultDiff() / 1
  end

  local left = math.abs(math.sin(x*x-y*y))
  local right = math.sin(x+y)+math.cos(x*y)

  if (math.abs(left - right) < self.diff) then return true end
  return false
end

qcp.startExpression(e4)

-----------------

local e5 = qcp.Expression:new {xLower=-20, xUpper=20, yLower=-20, yUpper=20, name = "有趣的图5", splitInPoint = 9, expression="x/sin(x) +- y/sin(y) = +- xy/sin(xy)"}
function e5:f(x, y)
  if (self.diff == nil) then
    self.diff = self:calcDefaultDiff() / 1
  end

  local l1 = x/math.sin(x) + y/math.sin(y)
  local l2 = x/math.sin(x) - y/math.sin(y)
  local r1 = x*y/math.sin(x*y)
  local r2 = -(x*y/math.sin(x*y))

  if (math.abs(l1-r1) < self.diff) or (math.abs(l2-r2) < self.diff) or (math.abs(l1-r2) < self.diff) or (math.abs(l2-r1) < self.diff)
    then return true end
  return false
end

qcp.startExpression(e5)

-----------------

local e6 = qcp.Expression:new {name = "有趣的图6", splitInPoint = 3, expression="cos(cos(min(sin(x)+y,x+sin(y)))) - cos(sin(max(sin(y)+x,y+sin(x)))) > 0"}
function e6:f(x, y)
  if (self.diff == nil) then
    self.diff = self:calcDefaultDiff() / 1
  end

  local l1 = math.cos(math.cos(math.min(math.sin(x)+y, x+math.sin(y))))
  local l2 = math.cos(math.sin(math.max(math.sin(y)+x, y+math.sin(x))))

  if (l1-l2 > 0)
    then return true end
  return false
end

qcp.startExpression(e6)

-----------------

local e7 = qcp.Expression:new {name = "有趣的图7", splitInPoint = 3, expression="sin((2^|y|)*x +- pi/4(y-|y|) - pi/2) = 0", xLower=-8, xUpper=8, yLower=-2, yUpper=6}
function e7:f(x, y)
  if (self.diff == nil) then
    self.diff = self:calcDefaultDiff() / 100
  end

  local l1 = (2^math.abs(y))*x
  local l2 = math.pi/4*(y-math.abs(y))
  local l3 = math.pi/2
  local s1=math.sin(l1+l2-l3)
  local s2=math.sin(l1-l2-l3)

  if (s1 < self.diff or s2 < self.diff)
    then return true end
  return false
end

qcp.startExpression(e7)

-----------------

local e8 = qcp.Expression:new {name = "温柔曲线", splitInPoint = 4, expression="y=3*x*log(x)-1.0/36*exp(-(36.0*x-36.0/exp(1))**4)", yLower=0, yUpper=1.2, xLower=-1.5, xUpper=0}
function e8:f(x, y)
  if (self.diff == nil) then
    self.diff = self:calcDefaultDiff() / 10
  end

  local l1 = x
  local r1 = 3*y*math.log(y)-1.0/36*math.exp(-(36.0*y-36/math.exp(1))^4)

  if (math.abs(l1-r1) < self.diff)
    then return true end
  return false
end

qcp.startExpression(e8)

