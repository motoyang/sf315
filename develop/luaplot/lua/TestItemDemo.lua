-- TestItemDemo.lua

--[[

This example shows how to create a text label that is always positioned at the top of the axis rect and an arrow that connects a point in plot coordinates with that label.

--]]

local pa=require("qcpplot.print_any")
local qcp=require("qcpplot.qcp")
local qt=require("qcpplot.qt5")

function fun4MW(mainWidget)
  local p = mainWidget:getPlot()
  fun4Plot(p)
end

function fun4Plot(p)
  -- add the text label at the top:
  local textLabel = p:createItemText(p);
  textLabel:setPositionAlignment(qt.AlignTop + qt.AlignHCenter);
  textLabel.position:setType(qcp.ItemPosition.ptAxisRectRatio);
  textLabel.position:setCoordsXY(0.5, 0); -- place position at center/top of axis rect
  textLabel:setText("Text Item Demo");
  textLabel:setFont(luaplot.FontConstructor.fromFamily(p:font():family(), 16, -1, false)); -- make font a bit larger
  textLabel:setPen(luaplot.PenConstructor.fromColor(luaplot.QColor.fromGlobal(qt.black))); -- show black border around text
 
  -- add the arrow:
  local arrow = p:createItemLine(p);
  arrow.start:setParentAnchor(textLabel.bottom);
  arrow.theEnd:setCoordsXY(4, 1.6); -- point to (4, 1.6) in x-y-plot coordinates
  arrow:setHead(luaplot.LineEnding(qcp.LineEnding.esSpikeArrow, 8, 10, false));
end

qcp.startPlot(fun4Plot)

qcp.startMainWindow(fun4MW)


