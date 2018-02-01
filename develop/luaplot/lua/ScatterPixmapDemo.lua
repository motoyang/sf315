-- ScatterPixmapDemo.lua

--[[

Pixmap scatter points and a multi-lined axis label, as well as a plot title at the top

--]]

--local ar=require("l1.array")
local pa=require("qcpplot.print_any")
local qcp=require("qcpplot.qcp")
local qt=require("qcpplot.qt5")

-- fp means function of plot
function fp(plot)

  plot:axisRect(0):setBackgroundByPixmap(luaplot.PixmapConstructor.fromFile("./solarpanels.png", nil, qt.AutoColor));
  plot:addGraph(nil, nil);
  plot:lastGraph():setLineStyle(qcp.Graph.lsLine);
  local pen = luaplot.QPen();
  pen:setColor(luaplot.ColorConstructor.fromRGB(255, 200, 20, 200));
  pen:setStyle(qt.DashLine);
  pen:setWidthF(2.5);
  plot:lastGraph():setPen(pen);
  plot:lastGraph():setBrush(luaplot.BrushConstructor.fromColor(luaplot.ColorConstructor.fromRGB(255,200,20,70), qt.SolidPattern));
  plot:lastGraph():setScatterStyle(luaplot.ScatterStyleConstructor.fromPixmap(luaplot.PixmapConstructor.fromFile("./sun.png", nil, qt.AutoColor):scaledXY(32, 32, qt.IgnoreAspectRatio, qt.FastTransformation)));
  -- set graph name, will show up in legend next to icon:
  plot:lastGraph():setName("Data from Photovoltaic\nenergy barometer 2011");
  -- set data:
  local year, value = {2005, 2006, 2007, 2008, 2009, 2010, 2011}, {2.17, 3.42, 4.94, 10.38, 15.86, 29.33, 52.1};
  plot:lastGraph():setVector(year, value);

  -- set title of plot:
  plot:plotLayout():insertRow(0);
  local textElement = plot:createTextElement(plot, "Regenerative Energies", luaplot.FontConstructor.fromFamily("sans", 12, qt.Font.Bold, falsee));
  plot:plotLayout():addElement(0, 0, textElement);
  -- axis configurations:
  plot.xAxis:setLabel("Year");
  plot.yAxis:setLabel("Installed Gigawatts of\nphotovoltaic in the European Union");
  plot.xAxis2:setVisible(true);
  plot.yAxis2:setVisible(true);
  plot.xAxis2:setTickLabels(false);
  plot.yAxis2:setTickLabels(false);
  plot.xAxis2:setTicks(false);
  plot.yAxis2:setTicks(false);
  plot.xAxis2:setSubTicks(false);
  plot.yAxis2:setSubTicks(false);
  plot.xAxis:setRange(2004.5, 2011.5);
  plot.yAxis:setRange(0, 52);
  -- setup legend:
  plot.legend:setFont(luaplot.FontConstructor.fromFamily(plot:font():family(), 7, -1, false));
  plot.legend:setIconSizeXY(50, 20);
  plot.legend:setVisible(true);
  plot:axisRect(0):insetLayout():setInsetAlignment(0, qt.AlignLeft + qt.AlignTop);

end

function fw(w)
  local p = w:getPlot()
  fp(p)
end

qcp.startMainWindow(fw)

