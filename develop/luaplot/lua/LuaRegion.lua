-- LuaRegion.lua

--[[
	这是一些关于Region的例子，使用的addFunction、addLogic和addEquation。
	注意：addEquation的计算量很大，作图有些慢。
--]]

local pa=require("qcpplot.print_any")
local qcp=require("qcpplot.qcp")
local qt=require("qcpplot.qt5")

--------------------

qcp.startPlot(
  function(p)
    local r = qcp.Region:new{plot=p, name="密集恐惧曲线1：y=cos(1/x)", xLower=-1.2, xUpper=1.2, yLower=-0.3, yUpper=0.3, }
    r:addFunction(
		  function (x)
			  return math.cos(1/x)
		  end
    )
  end
)

--------------------

function fp01(p)
  local r = qcp.Region:new{plot=p, name="密集恐惧曲线2：y=x*sin(x^2)", xLower=-10, xUpper=10, yLower=-10, yUpper=10, }
  r:addFunction(
		function (x)
			return x * math.sin(x ^ 2)
		end
  )
end
qcp.startPlot(fp01)

--------------------

function fp02(p)
  local r = qcp.Region:new{plot=p, name="爱心曲线：y=sqrt(1-(abs(x)-1)^2) || y=acos(1-abs(x))-pi", xLower=-3.5, xUpper=3.5, yLower=-3.5, yUpper=1.5, }
  r:addFunction(
		function (x)
			return math.sqrt(1-(math.abs(x)-1)^2)
		end
  )
  r:addFunction(
		function (x)
			return math.acos(1-math.abs(x))-math.pi
		end
  )
end
qcp.startPlot(fp02)

--------------------
--[[
qcp.startPlot(
  function (p)
    local r = qcp.Region:new{plot=p, name="爱心曲线：x*x+(y-(x*x)^(1/3))^2=1", xLower=-2, xUpper=2, yLower=-1.5, yUpper=2, }
    r:addEquation(
      function (x, y)
        local r = 1
        local l = x*x + (y - (x*x)^(1/3))^2
        return l, r
      end
    )
  end
)
--]]
--------------------

qcp.startPlot(
  function (p)
    local r = qcp.Region:new{plot=p, name=" 污污污曲线：y=x^(sin(x^cosx))", xLower=-20, xUpper=20, yLower=-20, yUpper=20, }
    r:addFunction(
      function (x)
        return x^math.sin(x^math.cos(x))
      end
    )
  end
)

--------------------

--[[
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

function fp07a(p)
  local r = qcp.Region:new{plot=p, name="最丑曲线：y=sin(1/x)", xLower=-1, xUpper=1, yLower=-1, yUpper=1, }
  r:addFunction(
		function (x)
			return math.sin(1/x)
		end
  )
end
qcp.startPlot(fp07a)

--------------------

function fp08(p)
  local r = qcp.Region:new{plot=p, yLower = -1.5, yUpper = 1.5, name="函数: y=sin(x), y=1.2*cos(x), y = -0.5x - 0.5"}

  local g1 = r:addFunction(math.sin)
  g1:setBrush(luaplot.QBrush.fromColor(luaplot.QColor.fromRGB(20, 20, 20, 20), qt.SolidPattern))

  local g2 = r:addFunction(function (x) return -0.5*x - 0.5 end)
  g2:setPen(luaplot.QPen.fromColor(luaplot.QColor.fromGlobal(qt.green)))

  local g3 = r:addFunction(function (x) return 1.2*math.cos(x) end)
  g3:setBrush(luaplot.QBrush.fromColor(luaplot.QColor.fromRGB(40, 40, 10, 20), qt.SolidPattern))

  local curve = r:addEquation(
    function (x, y)
      local r = y
      local l1 = math.sin(x)
      local l2 = 1.2*math.cos(x)
      local v = math.max(math.abs(l1-r), math.abs(l2-r))
      return v, 0
    end
  )
  curve:setPen(luaplot.QPen.fromColor(luaplot.QColor.fromGlobal(qt.red)))
  curve:setScatterStyle(luaplot.ScatterStyle.fromShapeAndSize(qcp.ScatterStyle.ssCircle, 16))
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
--]]
