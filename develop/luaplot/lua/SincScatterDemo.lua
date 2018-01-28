-- SincScatterDemo.lua

--[[

sinc function with data points, corresponding error bars and a 2-sigma confidence band.

--]]

--local ar=require("l1.array")
local pa=require("qcpplot.print_any")
local qcp=require("qcpplot.qcp")
local qt=require("qcpplot.qt5")

-- fp means function of plot
function fp(plot)

  plot.legend:setVisible(true);
  plot.legend:setFont(luaplot.FontConstructor.fromFamily("Helvetica",9,-1,false));
  -- set locale to english, so we get english decimal separator:
  plot:setLocale(luaplot.LocaleConstructor.fromLanguageAndCountry(qt.Locale.English, qt.Locale.UnitedKingdom));
  -- add confidence band graphs:
  plot:addGraph(nil, nil);
  local pen = luaplot.QPen();
  pen:setStyle(qt.DotLine);
  pen:setWidth(1);
  pen:setColor(luaplot.ColorConstructor.fromRGB(180,180,180,255));
  plot:graph(0):setName("Confidence Band 68%");
  plot:graph(0):setPen(pen);
  plot:graph(0):setBrush(luaplot.BrushConstructor.fromColor(luaplot.ColorConstructor.fromRGB(255,50,30,20), qt.SolidPattern));
  plot:addGraph(nil, nil);
  plot.legend:removeItemByIndex(plot.legend:itemCount()-1); -- don't show two confidence band graphs in legend
  plot:graph(1):setPen(pen);
  plot:graph(0):setChannelFillGraph(plot:graph(1));

  -- add theory curve graph:
  plot:addGraph(nil, nil);
  pen:setStyle(qt.DashLine);
  pen:setWidth(2);
  pen:setColor(luaplot.ColorConstructor.fromGlobal(qt.red));
  plot:graph(2):setPen(pen);
  plot:graph(2):setName("Theory Curve");
  -- add data point graph:
  plot:addGraph(nil, nil);
  plot:graph(3):setPen(luaplot.PenConstructor.fromColor(luaplot.ColorConstructor.fromGlobal(qt.blue)));
  plot:graph(3):setLineStyle(qcp.Graph.lsNone);
  plot:graph(3):setScatterStyle(luaplot.ScatterStyleConstructor.fromShapeAndSize(qcp.ScatterStyle.ssCross, 4));
  -- add error bars:
  local errorBars = plot:createErrorBars(plot.xAxis, plot.yAxis);
  errorBars:removeFromLegend();
  errorBars:setAntialiased(false);
  errorBars:setDataPlottable(plot:graph(3));
  errorBars:setPen(luaplot.PenConstructor.fromColor(luaplot.ColorConstructor.fromRGB(180,180,180,255)));
  plot:graph(3):setName("Measurement");

  -- generate ideal sinc curve data and some randomly perturbed data for scatter plot:
  local x0, y0 = {}, {};
  local yConfUpper, yConfLower = {}, {};
  for i=0, 250-1 do
    x0[i+1] = (i/249.0-0.5)*30+0.01; -- by adding a small offset we make sure not do divide by zero in next code line
    y0[i+1] = math.sin(x0[i+1])/x0[i+1]; -- sinc function
    yConfUpper[i+1] = y0[i+1]+0.15;
    yConfLower[i+1] = y0[i+1]-0.15;
    x0[i+1] = x0[i+1]*1000;
  end
  local x1, y1, y1err = {}, {}, {};
  for i=0, 50-1 do
    -- generate a gaussian distributed random number:
    local tmp1 = math.random();
    local tmp2 = math.random();
    local r = math.sqrt(-2*math.log(tmp1))*math.cos(2*math.pi*tmp2); -- box-muller transform for gaussian distribution
    -- set y1 to value of y0 plus a random gaussian pertubation:
    x1[i+1] = (i/50.0-0.5)*30+0.25;
    y1[i+1] = math.sin(x1[i+1])/x1[i+1]+r*0.15;
    x1[i+1] = x1[i+1]*1000;
    y1err[i+1] = 0.15;
  end
  -- pass data to graphs and let QCustomPlot determine the axes ranges so the whole thing is visible:
  plot:graph(0):setVector(x0, yConfUpper);
  plot:graph(1):setVector(x0, yConfLower);
  plot:graph(2):setVector(x0, y0);
  plot:graph(3):setVector(x1, y1);
  errorBars:setVector1(y1err);
  plot:graph(2):rescaleAxes();
  plot:graph(3):rescaleAxes(true);
  -- setup look of bottom tick labels:
  plot.xAxis:setTickLabelRotation(30);
  plot.xAxis:ticker():setTickCount(9);
  plot.xAxis:setNumberFormat("ebc");
  plot.xAxis:setNumberPrecision(1);
  plot.xAxis:moveRange(-10);
  -- make top right axes clones of bottom left axes. Looks prettier:
  plot:axisRect(0):setupFullAxesBox();

end

function fw(w)
  local p = w:getPlot()
  fp(p)
end

qcp.startMainWindow(fw)

