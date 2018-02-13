-- LuaRegion.lua

--[[

--]]

local pa=require("qcpplot.print_any")
local qcp=require("qcpplot.qcp")
local qt=require("qcpplot.qt5")

--------------------

function fp1(p)
  local r = qcp.Region:new{plot=p, yLower = -1.5, yUpper = 1.5,}
  r:addFunction(math.sin)
  r:addFunction(function (x) return math.cos(x) end)
end

function fp2(p)
  local r = qcp.Region:new{plot=p,}
  r:addLogic(
    function (x, y) 
      if (math.abs(math.sqrt(x^2+y^2)-8)<0.01 or math.sqrt(x^2+y^2)<8 and x<=0 and math.sqrt(x^2+(y-4)^2)>4 or math.sqrt(x^2+(y+4)^2)<4 or math.sqrt(x^2+(y-4)^2)<1) and math.sqrt(x^2+(y+4)^2)>1
      then return true end
      return false
    end
  )

end

function fp3(p)
  local r = qcp.Region:new{plot=p,}
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


function fp5(p)
  local r = qcp.Region:new{plot=p,}

  r:addEquation(
    function (x, y) 
      local l1 = x/math.sin(x) + y/math.sin(y)
      local r1 = x*y/math.sin(x*y)
      return l1, r1
    end
  )
  r:addEquation(
    function (x, y) 
      local l2 = x/math.sin(x) - y/math.sin(y)
      local r2 = -(x*y/math.sin(x*y))
      return l2, r2
    end
  )
end

qcp.startPlot(fp5)

--------------------