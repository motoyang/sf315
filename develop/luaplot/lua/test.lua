-- LuaRegion.lua

--[[
	这是一些关于Region的例子，使用的addFunction、addLogic和addEquation。
	注意：addEquation的计算量很大，作图有些慢。
--]]

local pa=require("qcpplot.print_any")
local qcp=require("qcpplot.qcp")
local qt=require("qcpplot.qt5")

local StatisticalBoxData = {
  key = 1.1,
  minimum = 2.2,
  lowerQuartile = 3.3,
  median = 4.4,
  upperQuartile = 5.5,
  maximum = 6.6,
};
StatisticalBoxData.outliers = {11.1, 22.2, 33.3, 44.4, 55.5}
pa.pprint("StatisticalBoxData=", StatisticalBoxData)

local a = luaplot.getStatisticalBoxData(StatisticalBoxData)
pa.pprint("a=", a)
--]]

--[[
--------------------
function fp01(p)
  local r = qcp.Region:new{plot=p, name="函数原型：sin(x^2 + y^2) = cos(xy)"}
  r:addEquation(
    function (x, y) 
      local left = math.sin(x*x + y*y)
      local right = math.cos(x*y)
      return left, right
    end
  )
end
qcp.startPlot(fp01)

function fp02(p)
  local r = qcp.Region:new{plot=p, name="函数原型：|sin(x^2 + 2xy)| = sin(x - 2y)"}
  r:addEquation(
    function (x, y) 
      local left = math.abs(math.sin(x*x+2*x*y))
      local right = math.sin(x-2*y)
      return left, right
    end
  )
end
qcp.startPlot(fp02)

function fp03(p)
  local r = qcp.Region:new{plot=p, name="函数原型：sin(sin(x) + cos(y)) = cos(sin(xy) + cos(x))"}
  r:addEquation(
    function (x, y) 
      local left = math.sin(math.sin(x) + math.cos(y))
      local right = math.cos(math.sin(x*y)+math.cos(x))
      return left, right
    end
  )
end
qcp.startPlot(fp03)

function fp04(p)
  local r = qcp.Region:new{plot=p, name="函数原型：|sin(x^2-y^2)|=sin(x+y)+cos(xy)"}
  r:addEquation(
    function (x, y) 
      local left = math.abs(math.sin(x*x-y*y))
      local right = math.sin(x+y)+math.cos(x*y)
      return left, right
    end
  )
end
qcp.startPlot(fp04)

--------------------

function fp05(p)
  local r = qcp.Region:new{plot=p, xLower=-100, xUpper=100, yLower=-100, yUpper=100, name="函数原型：(x^2 + y^2) % 80 > 5",}
  r:addLogic(
		function (x, y)
      local left1 = (x*x+y*y)%80
      local right1 = 5
    
      if (left1 < right1) 
      then return true end
      return false
		end
  )
end
qcp.startPlot(fp05)

--------------------

function fp06(p)
  local r = qcp.Region:new{plot=p, name="函数原型：cos(cos(min(sin(x)+y,x+sin(y)))) - cos(sin(max(sin(y)+x,y+sin(x)))) > 0",}
  r:addLogic(
		function (x, y)
      local l1 = math.cos(math.cos(math.min(math.sin(x)+y, x+math.sin(y))))
      local l2 = math.cos(math.sin(math.max(math.sin(y)+x, y+math.sin(x))))
    	if (l1 - l2 > 0) then return true end
    	return false
		end
  )
end
qcp.startPlot(fp06)

--------------------

function fp07(p)
  local r = qcp.Region:new{plot=p, name="温柔曲线：y=3*x*log(x)-1.0/36*exp(-(36.0*x-36.0/exp(1))**4)", xLower=0, xUpper=1.2, yLower=-1.5, yUpper=0, }
  r:addFunction(
		function (x)
			return 3*x*math.log(x)-1.0/36*math.exp(-(36.0*x-36/math.exp(1))^4)
		end
  )
end
qcp.startPlot(fp07)

--------------------

function fp08(p)
  local r = qcp.Region:new{plot=p, yLower = -1.5, yUpper = 1.5, name="函数: y=sin(x), y=1.2*cos(x), y = -0.5x - 0.5"}

  local g1 = r:addFunction(math.sin)
  g1:setBrush(luaplot.BrushConstructor.fromColor(luaplot.ColorConstructor.fromRGB(20, 20, 20, 20), qt.SolidPattern))

  local g2 = r:addFunction(function (x) return -0.5*x - 0.5 end)
  g2:setPen(luaplot.PenConstructor.fromColor(luaplot.ColorConstructor.fromGlobal(qt.green)))

  local g3 = r:addFunction(function (x) return 1.2*math.cos(x) end)
  g3:setBrush(luaplot.BrushConstructor.fromColor(luaplot.ColorConstructor.fromRGB(40, 40, 10, 20), qt.SolidPattern))

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
end
qcp.startPlot(fp08)

--------------------

function fp09(p)
  local r = qcp.Region:new{plot=p, name="太极图",}
  r:addLogic(
    function (x, y) 
      if (math.abs(math.sqrt(x^2+y^2)-8)<0.01 or math.sqrt(x^2+y^2)<8 and x<=0 and math.sqrt(x^2+(y-4)^2)>4 or math.sqrt(x^2+(y+4)^2)<4 or math.sqrt(x^2+(y-4)^2)<1) and math.sqrt(x^2+(y+4)^2)>1
      then return true end
      return false
    end
  )
end
qcp.startPlot(fp09)

--------------------

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

--------------------

function fp10(p)
  local r = qcp.Region:new{plot=p,}
  r:addEquation(
    function (x, y) 
      local left = math.exp(math.sin(x)+math.cos(y))
      local right = math.sin(math.exp(x+y))
      return left, right
    end
  )
end
qcp.startPlot(fp10)

--------------------

function fp11(p)
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
qcp.startPlot(fp11)

--------------------
--]]