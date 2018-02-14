-- LogarithmicAxisDemo.lua

--[[

Logarithmic axis scaling. Note correct display of the sine function crossing zero in negative infinity.

--]]

local pa=require("qcpplot.print_any")
local qcp=require("qcpplot.qcp")
local qt=require("qcpplot.qt5")

-- fp means function of plot
function fp(plot)
  plot:setNoAntialiasingOnDrag(true); -- more performance/responsiveness during dragging
  plot:addGraph(nil, nil);
  local pen = luaplot.QPen();
  pen:setColor(luaplot.ColorConstructor.fromRGB(255, 170, 100, 255));
  pen:setWidth(2);
  pen:setStyle(qt.DotLine);
  plot:graph(0):setPen(pen);
  plot:graph(0):setName("x");

  plot:addGraph(nil, nil);
  plot:graph(1):setPen(luaplot.PenConstructor.fromColor(luaplot.ColorConstructor.fromGlobal(qt.red)));
  plot:graph(1):setBrush(luaplot.BrushConstructor.fromColor(luaplot.ColorConstructor.fromRGB(255, 0, 0, 20), qt.SolidPattern));
  plot:graph(1):setName("-sin(x)exp(x)");

  plot:addGraph(nil, nil);
  plot:graph(2):setPen(luaplot.PenConstructor.fromColor(luaplot.ColorConstructor.fromGlobal(qt.blue)));
  plot:graph(2):setBrush(luaplot.BrushConstructor.fromColor(luaplot.ColorConstructor.fromRGB(0, 0, 255, 20), qt.SolidPattern));
  plot:graph(2):setName(" sin(x)exp(x)");

  plot:addGraph(nil, nil);
  pen:setColor(luaplot.ColorConstructor.fromRGB(0,0,0,255));
  pen:setWidth(1);
  pen:setStyle(qt.DashLine);
  plot:graph(3):setPen(pen);
  plot:graph(3):setBrush(luaplot.BrushConstructor.fromColor(luaplot.ColorConstructor.fromRGB(0,0,0,15), qt.SolidPattern));
  plot:graph(3):setLineStyle(qcp.Graph.lsStepCenter);
  plot:graph(3):setName("x!");

  local dataCount = 200;
  local dataFactorialCount = 21;
  local dataLinear, dataMinusSinExp, dataPlusSinExp, dataFactorial = {}, {}, {}, {};
  for i=0, dataCount-1 do
    dataLinear[i+1] = {}
    dataLinear[i+1].key = i/10.0;
    dataLinear[i+1].value = dataLinear[i+1].key;
    dataMinusSinExp[i+1] = {}
    dataMinusSinExp[i+1].key = i/10.0;
    dataMinusSinExp[i+1].value = -math.sin(dataMinusSinExp[i+1].key)*math.exp(dataMinusSinExp[i+1].key);
    dataPlusSinExp[i+1] = {}
    dataPlusSinExp[i+1].key = i/10.0;
    dataPlusSinExp[i+1].value = math.sin(dataPlusSinExp[i+1].key)*math.exp(dataPlusSinExp[i+1].key);
  end

  for i=0, dataFactorialCount-1 do
    dataFactorial[i+1] = {}
    dataFactorial[i+1].key = i;
    dataFactorial[i+1].value = 1.0;
    for k=1, i do dataFactorial[i+1].value = dataFactorial[i+1].value*k end; -- factorial
  end
  plot:graph(0):data():setVector(dataLinear);
  plot:graph(1):data():setVector(dataMinusSinExp);
  plot:graph(2):data():setVector(dataPlusSinExp);
  plot:graph(3):data():setVector(dataFactorial);

  plot.yAxis:grid():setSubGridVisible(true);
  plot.xAxis:grid():setSubGridVisible(true);
  plot.yAxis:setScaleType(qcp.Axis.stLogarithmic);
  plot.yAxis2:setScaleType(qcp.Axis.stLogarithmic);
  local logTicker = plot:createAxisTickerLog();
  local logTicker2 = plot:createAxisTickerLog();
  plot.yAxis:setTicker(logTicker);
  plot.yAxis2:setTicker(logTicker2);
  plot.yAxis:setNumberFormat("eb"); -- e = exponential, b = beautiful decimal powers
  plot.yAxis:setNumberPrecision(0); -- makes sure "1*10^4" is displayed only as "10^4"
  plot.xAxis:setRange(0, 19.9);
  plot.yAxis:setRange(1e-2, 1e10);

  -- make top right axes clones of bottom left axes:
  plot:axisRect(0):setupFullAxesBox();
  -- connect signals so top and right axes move in sync with bottom and left axes:
  luaplot.QObject.connect(plot.xAxis, qt.SIGNAL("rangeChanged(QCPRange)"), plot.xAxis2, qt.SLOT("setRange(QCPRange)"), qt.AutoConnection);
  luaplot.QObject.connect(plot.yAxis, qt.SIGNAL("rangeChanged(QCPRange)"), plot.yAxis2, qt.SLOT("setRange(QCPRange)"), qt.AutoConnection);

  plot.legend:setVisible(true);
  plot.legend:setBrush(luaplot.BrushConstructor.fromColor(luaplot.ColorConstructor.fromRGB(255,255,255,150), qt.SolidPattern));
  plot:axisRect(0):insetLayout():setInsetAlignment(0, qt.AlignLeft + qt.AlignTop); -- make legend align in top left corner or axis rect
end

function fw(w)
  local p = w:getPlot()
  fp(p)
end

qcp.startMainWindow(fw)

