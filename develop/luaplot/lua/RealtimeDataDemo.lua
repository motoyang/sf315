-- RealtimeDataDemo.lua

--[[

Real time generated data and time bottom axis

--]]

local pa=require("qcpplot.print_any")
local qcp=require("qcpplot.qcp")
local qt=require("qcpplot.qt5")

-- fp means function of plot
local function fp(plot)
  plot:addGraph(nil, nil); -- blue line
  plot:graph(0):setPen(luaplot.QPen.fromColor(luaplot.QColor.fromRGB(40, 110, 255, 255)));
  plot:addGraph(nil, nil); -- red line
  plot:graph(1):setPen(luaplot.QPen.fromColor(luaplot.QColor.fromRGB(255, 110, 40, 255)));
   
  local timeTicker = plot:createAxisTickerTime();
  timeTicker:setTimeFormat("%h:%m:%s");
  plot.xAxis:setTicker(timeTicker);
  plot:axisRect(0):setupFullAxesBox();
  plot.yAxis:setRange(-1.2, 1.2);
   
  -- make left and bottom axes transfer their ranges to right and top axes:
  luaplot.QObject.connect(plot.xAxis, qt.SIGNAL("rangeChanged(QCPRange)"), plot.xAxis2, qt.SLOT("setRange(QCPRange)"), qt.AutoConnection);
  luaplot.QObject.connect(plot.yAxis, qt.SIGNAL("rangeChanged(QCPRange)"), plot.yAxis2, qt.SLOT("setRange(QCPRange)"), qt.AutoConnection);
end

local function fw(w)
  local p = w:getPlot()
  fp(p)
end

local lastFpsKey = 0;
local frameCount = 0;
local time = luaplot.QTime.currentTime();
local lastPointKey = 0;
function timerSlot(plot)
  local key = time:elapsed() / 1000.0;
  if (key - lastPointKey > 0.002) then 
    plot:graph(0):addData(key, math.sin(key)+math.random()*1*math.sin(key/0.3843));
    plot:graph(1):addData(key, math.cos(key)+math.random()*0.5*math.sin(key/0.4364));

    lastPointKey = key;

    local r = plot.xAxis:range()
    plot:graph(0):data():removeBefore(r.lower)
    plot:graph(1):data():removeBefore(r.lower)
  end

  -- make key axis range scroll with the data (at a constant range size of 8):
  plot.xAxis:setRangeWithSize(key, 8, qt.AlignRight);
  plot:replot(qcp.CustomPlot.rpRefreshHint);

--  local key = secs
  frameCount = frameCount + 1
  if (key - lastFpsKey >= 1) then
    local size = plot:graph(0):data():size() + plot:graph(1):data():size();
    local fps = frameCount/(key-lastFpsKey)
    lastFpsKey = key
    frameCount = 0

    local msg = string.format("%0.0f FPS, Total Data points: %d", fps, size)
    return msg
  end
end

qcp.startMainWindow(fw, "timerSlot", 0)

