-- LuaExpression1.lua

--[[


--]]

--local ar=require("l1.array")
local pa=require("qcpplot.print_any")
local qcp=require("qcpplot.qcp")
local qt=require("qcpplot.qt5")

Expression = {
  name = "name of the expression",
  expression = "fun_e1",
  xLower = -10, xUpper = 10,
  yLower = -10, yUpper = 10,
  pointsOfWidth = 800, pointsOfHeight = 600,
  splitInPoint = 4,
};
  
function Expression:new(e)
    e = e or {}
    setmetatable(e, self)
    self.__index = self
    return e
  end

function Expression:diff()
    local dx = (self.xUpper - self.xLower) / self.pointsOfWidth;
    local dy = (self.yUpper - self.yLower) / self.pointsOfHeight;

    return math.sqrt(dx*dx + dy*dy)
  end

local function defaultDiff(e)
  local dx = (e.xUpper - e.xLower) / e.pointsOfWidth;
  local dy = (e.yUpper - e.yLower) / e.pointsOfHeight;

  return math.sqrt(dx*dx + dy*dy)
end

local diff = 0.0
function fun_e1(x, y)

  local left = math.sin(x*x + y*y)
  local right = math.cos(x*y)

  if (math.abs(left - right) < diff) then return true end
  return false
  
end

-- fp means function of plot
function fp(plot)
--[[
  local e1 = {}
  e1.name = "expression1 name"
  e1.expression = "fun_e1"
  e1.xLower, e1.xUpper = -10, 10
  e1.yLower, e1.yUpper = -10, 10
  e1.pointsOfWidth, e1.pointsOfHeight = 1000, 800
  e1.splitInPoint = 5

  diff = defaultDiff(e1)
--]]

  local e = Expression:new {splitInPoint = 6}
  local d = e:diff()
  print(d)

  --plot:addLuaExpression(e)

end

function fw(w)
  local p = w:getPlot()
  fp(p)
end

qcp.startMainWindow(fw)

