-- StyledPlotDemo.lua

--[[

Demonstrating QCustomPlot's versatility in styling the plot

--]]

local pa=require("qcpplot.print_any")
local qcp=require("qcpplot.qcp")
local qt=require("qcpplot.qt5")

-- fp means function of plot
function fp(plot)
  -- prepare data:
  local x1, y1 = {}, {};
  local x2, y2 = {}, {};
  local x3, y3 = {}, {};
  local x4, y4 = {}, {};
  for i=0, 20-1 do
    x1[i+1] = i/(20-1)*10;
    y1[i+1] = math.cos(x1[i+1]*0.8+math.sin(x1[i+1]*0.16+1.0))*math.sin(x1[i+1]*0.54)+1.4;
  end
  
  for i=0, 100-1 do
    x2[i+1] = i/(100-1)*10;
    y2[i+1] = math.cos(x2[i+1]*0.85+math.sin(x2[i+1]*0.165+1.1))*math.sin(x2[i+1]*0.50)+1.7;
  end
  
  for i=0, 20-1 do
    x3[i+1] = i/(20-1)*10;
    y3[i+1] = 0.05+3*(0.5+math.cos(x3[i+1]*x3[i+1]*0.2+2)*0.5)/(x3[i+1]+0.7)+math.random()*0.01;
  end
  
  for i=0, 20-1 do
    x4[i+1] = x3[i+1];
    y4[i+1] = (0.5-y3[i+1])+((x4[i+1]-2)*(x4[i+1]-2)*0.02);
  end
   
  -- create and configure plottables:
  local graph1 = plot:addGraph(nil, nil);
  graph1:setVector(x1, y1);
  graph1:setScatterStyle(
    luaplot.ScatterStyleConstructor.fromShapePenBrushAndSize(
      qcp.ScatterStyle.ssCircle,
      luaplot.PenConstructor.fromBrush(luaplot.BrushConstructor.fromColor(luaplot.ColorConstructor.fromGlobal(qt.black), qt.SolidPattern), 1.5, qt.SolidLine, qt.SquareCap, qt.BevelJoin),
      luaplot.BrushConstructor.fromColor(luaplot.ColorConstructor.fromGlobal(qt.white), qt.SolidPattern),
      9
    )
  );
  
  graph1:setPen(
    luaplot.PenConstructor.fromBrush(
      luaplot.BrushConstructor.fromColor(
        luaplot.ColorConstructor.fromRGB(120, 120, 120, 255),
        qt.SolidPattern
      ),
    2, qt.SolidLine, qt.SquareCap, qt.BevelJoin
    )
  );
   
  local graph2 = plot:addGraph(nil, nil);
  graph2:setVector(x2, y2);
  graph2:setPen(luaplot.PenConstructor.fromStyle(qt.NoPen));
  graph2:setBrush(luaplot.BrushConstructor.fromColor(luaplot.ColorConstructor.fromRGB(200, 200, 200, 20), qt.SolidPattern));
  graph2:setChannelFillGraph(graph1);
   
  local bars1 = plot:createBars(plot.xAxis, plot.yAxis);
  bars1:setWidth(9/20.0);
  bars1:setVector(x3, y3);
  bars1:setPen(luaplot.PenConstructor.fromStyle(qt.NoPen));
  bars1:setBrush(luaplot.BrushConstructor.fromColor(luaplot.ColorConstructor.fromRGB(10, 140, 70, 160), qt.SolidPattern));
   
  local bars2 = plot:createBars(plot.xAxis, plot.yAxis);
  bars2:setWidth(9/20);
  bars2:setVector(x4, y4);
  bars2:setPen(luaplot.PenConstructor.fromStyle(qt.NoPen));
  bars2:setBrush(luaplot.BrushConstructor.fromColor(luaplot.ColorConstructor.fromRGB(10, 100, 50, 70), qt.SolidPattern));
  bars2:moveAbove(bars1);
   
  -- move bars above graphs and grid below bars:
  plot:addLayer("abovemain", plot:layerByName("main"), qcp.CustomPlot.limAbove);
  plot:addLayer("belowmain", plot:layerByName("main"), qcp.CustomPlot.limBelow);
  graph1:setLayer("abovemain");
  plot.xAxis:grid():setLayer("belowmain");
  plot.yAxis:grid():setLayer("belowmain");
   
  -- set some pens, brushes and backgrounds:
  local pen1 = luaplot.PenConstructor.fromBrush(
    luaplot.BrushConstructor.fromColor(
      luaplot.ColorConstructor.fromRGB(120, 120, 120, 255),
      qt.SolidPattern
    ),
    2, qt.SolidLine, qt.SquareCap, qt.BevelJoin
  );
  plot.xAxis:setBasePen(pen1);
  plot.yAxis:setBasePen(pen1);
  plot.xAxis:setTickPen(pen1);
  plot.yAxis:setTickPen(pen1);
  plot.xAxis:setSubTickPen(pen1);
  plot.yAxis:setSubTickPen(pen1);
  plot.xAxis:setTickLabelColor(luaplot.ColorConstructor.fromGlobal(qt.white));
  plot.yAxis:setTickLabelColor(luaplot.ColorConstructor.fromGlobal(qt.white));
  
  pen1 = luaplot.PenConstructor.fromBrush(
      luaplot.BrushConstructor.fromColor(
        luaplot.ColorConstructor.fromRGB(140, 140, 140, 255),
        qt.SolidPattern
      ),
    2, qt.DotLine, qt.SquareCap, qt.BevelJoin
  );
  plot.xAxis:grid():setPen(pen1);
  plot.yAxis:grid():setPen(pen1);
  
  pen1 = luaplot.PenConstructor.fromBrush(
    luaplot.BrushConstructor.fromColor(
        luaplot.ColorConstructor.fromRGB(80, 80, 80, 255),
        qt.SolidPattern
      ),
    2, qt.DotLine, qt.SquareCap, qt.BevelJoin
  );
  plot.xAxis:grid():setSubGridPen(pen1);
  plot.yAxis:grid():setSubGridPen(pen1);
  plot.xAxis:grid():setSubGridVisible(true);
  plot.yAxis:grid():setSubGridVisible(true);
  plot.xAxis:grid():setZeroLinePen(luaplot.PenConstructor.fromStyle(qt.NoPen));
  plot.yAxis:grid():setZeroLinePen(luaplot.PenConstructor.fromStyle(qt.NoPen));
  plot.xAxis:setUpperEnding(luaplot.LineEnding(qcp.LineEnding.esSpikeArrow, 8, 10, false));
  plot.yAxis:setUpperEnding(luaplot.LineEnding(qcp.LineEnding.esSpikeArrow, 8, 10, false));
  
  local plotGradient = luaplot.QLinearGradient();
  plotGradient:setStartXY(0, 0);
  plotGradient:setFinalStopXY(0, 350);
  plotGradient:setColorAt(0, luaplot.ColorConstructor.fromRGB(80, 80, 80, 255));
  plotGradient:setColorAt(1, luaplot.ColorConstructor.fromRGB(50, 50, 50, 255));
  plot:setBackground(luaplot.BrushConstructor.fromGradient(plotGradient));
  
  local axisRectGradient = luaplot.QLinearGradient();
  axisRectGradient:setStartXY(0, 0);
  axisRectGradient:setFinalStopXY(0, 350);
  axisRectGradient:setColorAt(0, luaplot.ColorConstructor.fromRGB(80, 80, 80, 255));
  axisRectGradient:setColorAt(1, luaplot.ColorConstructor.fromRGB(30, 30, 30, 255));
  plot:axisRect(0):setBackgroundByBrush(luaplot.BrushConstructor.fromGradient(axisRectGradient));
   
  plot:rescaleAxes();
  plot.yAxis:setRange(0, 2);
end

function fw(w)
  local p = w:getPlot()
  fp(p)
end

qcp.startMainWindow(fw)

