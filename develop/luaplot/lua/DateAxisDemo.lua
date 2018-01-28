-- DateAxisDemo.lua

--[[

Random walks with fill and smart date ticks on the bottom axis

--]]

--local ar=require("l1.array")
local pa=require("qcpplot.print_any")
local qcp=require("qcpplot.qcp")
local qt=require("qcpplot.qt5")

-- fp means function of plot
function fp(plot)


  -- set locale to english, so we get english month names:
  plot:setLocale(luaplot.LocaleConstructor.fromLanguageAndCountry(qt.Locale.English, qt.Locale.UnitedKingdom))

  -- seconds of current time, we'll use it as starting point in time for data:
  local now = luaplot.QDateTime.currentDateTime():toTime_t()
  math.randomseed(8)      -- set the random seed, so we always get the same random data
  -- create multiple graphs:
  for gi=0, 5-1 do
--    print("gi=", gi, "#1", 20+200/4.0*gi, "#2=", 70*(1.6-gi/4.0))
    plot:addGraph(nil, nil)
    local color = luaplot.ColorConstructor.fromRGB(math.floor(20+200/4.0*gi+0.5), math.floor(70*(1.6-gi/4.0)+0.5), 150, 150)
    plot:lastGraph():setLineStyle(qcp.Graph.lsLine)
    plot:lastGraph():setPen(luaplot.PenConstructor.fromColor(color:lighter(200)))
    plot:lastGraph():setBrush(luaplot.BrushConstructor.fromColor(color, qt.SolidPattern))
    -- generate random walk data:
    local timeData = {}
    for i=0, 250-1 do
      timeData[i+1] = {}
      timeData[i+1].key = now + 24*3600*i
      if i == 0 then
        timeData[i+1].value = (i/50.0+1)*(math.random()-0.5)
      else
        timeData[i+1].value = math.abs(math.abs(timeData[i].value)*(1+0.02/4.0*(4-gi)) + (i/50.0+1)*(math.random()-0.5))
      end
    end
    plot:lastGraph():data():setVector(timeData)
  end


  -- configure bottom axis to show date instead of number:
  local dateTicker = plot:createAxisTickerDateTime()
  dateTicker:setDateTimeFormat("d. MMMM\nyyyy")
  plot.xAxis:setTicker(dateTicker)
  -- configure left axis text labels:
  local textTicker = plot:createAxisTickerText()
  textTicker:addTick(10, "a bit\nlow")
    textTicker:addTick(50, "quite\nhigh")
    plot.yAxis:setTicker(textTicker)
  -- set a more compact font size for bottom and left axis tick labels:
  plot.xAxis:setTickLabelFont(luaplot.FontConstructor.fromFamily(luaplot.QFont():family(), 8, -1, false))
  plot.yAxis:setTickLabelFont(luaplot.FontConstructor.fromFamily(luaplot.QFont():family(), 8, -1, false))
  --  set axis labels:
  plot.xAxis:setLabel("Date")
  plot.yAxis:setLabel("Random wobbly lines value")
  -- make top and right axes visible but without ticks and labels:
  plot.xAxis2:setVisible(true)
  plot.yAxis2:setVisible(true)
  plot.xAxis2:setTicks(false)
  plot.yAxis2:setTicks(false)
  plot.xAxis2:setTickLabels(false)
  plot.yAxis2:setTickLabels(false)
  -- set axis ranges to show all data:
  plot.xAxis:setRange(now, now+24*3600*249)
  plot.yAxis:setRange(0, 60)
  -- show legend with slightly transparent background brush:
  plot.legend:setVisible(true)
  plot.legend:setBrush(luaplot.BrushConstructor.fromColor(luaplot.ColorConstructor.fromRGB(255, 255, 255, 150), qt.SolidPattern))

end

function fw(w)
  local p = w:getPlot()
  fp(p)
end

qcp.startMainWindow(fw)

