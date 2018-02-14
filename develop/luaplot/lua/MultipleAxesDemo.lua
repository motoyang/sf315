-- MultipleAxesDemo.lua

--[[

Real time generated data and time bottom axis

--]]

local pa=require("qcpplot.print_any")
local qcp=require("qcpplot.qcp")
local qt=require("qcpplot.qt5")

-- fp means function of plot
function fp(plot)
  plot:setLocale(luaplot.LocaleConstructor.fromLanguageAndCountry(qt.Locale.English, qt.Locale.UnitedKingdom)); -- period as decimal separator and comma as thousand separator
  plot.legend:setVisible(true);
  local legendFont = luaplot.FontConstructor.fromFont(plot:font());  -- start out with MainWindow's font..
  legendFont:setPointSize(9); -- and make a bit smaller for legend
  plot.legend:setFont(legendFont);
  plot.legend:setBrush(luaplot.BrushConstructor.fromColor(luaplot.ColorConstructor.fromRGB(255,255,255,230), qt.SolidPattern));

  -- by default, the legend is in the inset layout of the main axis rect. So this is how we access it to change legend placement:
  plot:axisRect(0):insetLayout():setInsetAlignment(0, qt.AlignBottom + qt.AlignRight);

  -- setup for graph 0: key axis left, value axis bottom
  -- will contain left maxwell-like function
  plot:addGraph(plot.yAxis, plot.xAxis);
  plot:graph(0):setPen(luaplot.PenConstructor.fromColor(luaplot.ColorConstructor.fromRGB(255, 100, 0, 255)));
  plot:graph(0):setBrush(luaplot.BrushConstructor.fromPixmap(luaplot.PixmapConstructor.fromFile("./sun.png", nil, qt.AutoColor):scaledXY(32, 32, qt.IgnoreAspectRatio, qt.FastTransformation))); -- fill with texture of specified image
  plot:graph(0):setLineStyle(qcp.Graph.lsLine);
  plot:graph(0):setScatterStyle(luaplot.ScatterStyleConstructor.fromShapeAndSize(qcp.ScatterStyle.ssDisc, 5));
  plot:graph(0):setName("Left maxwell function");

  -- setup for graph 1: key axis bottom, value axis left (those are the default axes)
  -- will contain bottom maxwell-like function with error bars
  plot:addGraph(nil, nil);
  plot:graph(1):setPen(luaplot.PenConstructor.fromColor(luaplot.ColorConstructor.fromGlobal(qt.red)));
  plot:graph(1):setBrush(luaplot.BrushConstructor.fromPixmap(luaplot.PixmapConstructor.fromFile("./sun.png", nil, qt.AutoColor):scaledXY(16, 16, qt.IgnoreAspectRatio, qt.FastTransformation)));
  plot:graph(1):setLineStyle(qcp.Graph.lsStepCenter);
  plot:graph(1):setScatterStyle(
    luaplot.ScatterStyleConstructor.fromShapePenBrushAndSize(
      qcp.ScatterStyle.ssCircle,
      luaplot.PenConstructor.fromColor(
        luaplot.ColorConstructor.fromGlobal(qt.red)
      ),
      luaplot.BrushConstructor.fromColor(
        luaplot.ColorConstructor.fromGlobal(qt.white),
        qt.SolidPattern
      ),
      7
    )
  );
  plot:graph(1):setName("Bottom maxwell function");
  local errorBars = plot:createErrorBars(plot.xAxis, plot.yAxis);
  errorBars:removeFromLegend();
  errorBars:setDataPlottable(plot:graph(1));

  -- setup for graph 2: key axis top, value axis right
  -- will contain high frequency sine with low frequency beating:
  plot:addGraph(plot.xAxis2, plot.yAxis2);
  plot:graph(2):setPen(luaplot.PenConstructor.fromColor(luaplot.ColorConstructor.fromGlobal(qt.blue)));
  plot:graph(2):setName("High frequency sine");

  -- setup for graph 3: same axes as graph 2
  -- will contain low frequency beating envelope of graph 2
  plot:addGraph(plot.xAxis2, plot.yAxis2);
  local blueDotPen = luaplot.QPen();
  blueDotPen:setColor(luaplot.ColorConstructor.fromRGB(30, 40, 255, 150));
  blueDotPen:setStyle(qt.DotLine);
  blueDotPen:setWidthF(4);
  plot:graph(3):setPen(blueDotPen);
  plot:graph(3):setName("Sine envelope");

  -- setup for graph 4: key axis right, value axis top
  -- will contain parabolically distributed data points with some random perturbance
  plot:addGraph(plot.yAxis2, plot.xAxis2);
  plot:graph(4):setPen(luaplot.PenConstructor.fromColor(luaplot.ColorConstructor.fromRGB(50, 50, 50, 255)));
  plot:graph(4):setLineStyle(qcp.Graph.lsNone);
  plot:graph(4):setScatterStyle(luaplot.ScatterStyleConstructor.fromShapeAndSize(qcp.ScatterStyle.ssCircle, 4));
  plot:graph(4):setName("Some random data around\na quadratic function");

  local x0, y0 = {}, {};
  local x1, y1, y1err = {}, {}, {};
  local x2, y2 = {}, {};
  local x3, y3 = {}, {};
  local x4, y4 = {}, {};
  for i=0, 25-1 do -- data for graph 0
    x0[i+1] = 3*i/25.0;
    y0[i+1] = math.exp(-x0[i+1]*x0[i+1]*0.8)*(x0[i+1]*x0[i+1]+x0[i+1]);
  end
  for i=0, 15-1 do -- data for graph 1
    x1[i+1] = 3*i/15.0;;
    y1[i+1] = math.exp(-x1[i+1]*x1[i+1])*(x1[i+1]*x1[i+1])*2.6;
    y1err[i+1] = y1[i+1]*0.25;
  end
  for i=0, 250-1 do -- data for graphs 2, 3 and 4
    x2[i+1] = i/250.0*3*math.pi;
    x3[i+1] = x2[i+1];
    x4[i+1] = i/250.0*100-50;
    y2[i+1] = math.sin(x2[i+1]*12)*math.cos(x2[i+1])*10;
    y3[i+1] = math.cos(x3[i+1])*10;
    y4[i+1] = 0.01*x4[i+1]*x4[i+1] + 1.5*(math.random()-0.5) + 1.5*math.pi;
  end

  -- pass data points to graphs:
  plot:graph(0):setVector(x0, y0);
  plot:graph(1):setVector(x1, y1);
  errorBars:setVector1(y1err);
  plot:graph(2):setVector(x2, y2);
  plot:graph(3):setVector(x3, y3);
  plot:graph(4):setVector(x4, y4);
  -- activate top and right axes, which are invisible by default:
  plot.xAxis2:setVisible(true);
  plot.yAxis2:setVisible(true);
  -- set ranges appropriate to show data:
  plot.xAxis:setRange(0, 2.7);
  plot.yAxis:setRange(0, 2.6);
  plot.xAxis2:setRange(0, 3.0*math.pi);
  plot.yAxis2:setRange(-70, 35);
  -- set pi ticks on top axis:
  plot.xAxis2:setTicker(plot:createAxisTickerPi())
  -- add title layout element:
  plot:plotLayout():insertRow(0);
  local textElement = plot:createTextElement(
    plot, 
    "Way too many graphs in one plot", 
    luaplot.FontConstructor.fromFamily("sans", 12, qt.Font.Bold)
  );
  plot:plotLayout():addElement(0, 0, textElement);
  -- set labels:
  plot.xAxis:setLabel("Bottom axis with outward ticks");
  plot.yAxis:setLabel("Left axis label");
  plot.xAxis2:setLabel("Top axis label");
  plot.yAxis2:setLabel("Right axis label");
  -- make ticks on bottom axis go outward:
  plot.xAxis:setTickLength(0, 5);
  plot.xAxis:setSubTickLength(0, 3);
  -- make ticks on right axis go inward and outward:
  plot.yAxis2:setTickLength(3, 3);
  plot.yAxis2:setSubTickLength(1, 1);
end

function fw(w)
  local p = w:getPlot()
  fp(p)
end

qcp.startMainWindow(fw)

