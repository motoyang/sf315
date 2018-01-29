-- ScatterStyleDemo.lua

--[[

A demonstration of several scatter point styles.

--]]

--local ar=require("l1.array")
local pa=require("qcpplot.print_any")
local qcp=require("qcpplot.qcp")
local qt=require("qcpplot.qt5")

-- fp means function of plot
function fp(plot)

    plot.legend:setVisible(true);
    plot.legend:setFont(luaplot.FontConstructor.fromFamily("Helvetica", 9, -1, false));
    plot.legend:setRowSpacing(-3);
    local shapes = {
        qcp.ScatterStyle.ssCross, 
        qcp.ScatterStyle.ssPlus,
        qcp.ScatterStyle.ssCircle,
        qcp.ScatterStyle.ssDisc,
        qcp.ScatterStyle.ssSquare,
        qcp.ScatterStyle.ssDiamond,
        qcp.ScatterStyle.ssStar,
        qcp.ScatterStyle.ssTriangle,
        qcp.ScatterStyle.ssTriangleInverted,
        qcp.ScatterStyle.ssCrossSquare,
        qcp.ScatterStyle.ssPlusSquare,
        qcp.ScatterStyle.ssCrossCircle,
        qcp.ScatterStyle.ssPlusCircle,
        qcp.ScatterStyle.ssPeace,
        qcp.ScatterStyle.ssCustom,
    };

    local pen = luaplot.QPen();
    -- add graphs with different scatter styles:
    for i=0, #shapes-1 do
      plot:addGraph(nil, nil);
      pen:setColor(luaplot.ColorConstructor.fromRGB(math.floor(0.5+math.sin(i*0.3)*100+100), math.floor(0.5+math.sin(i*0.6+0.7)*100+100), math.floor(0.5+math.sin(i*0.4+0.6)*100+100), 255));

      -- generate data:
      local x, y = {}, {};
      for k=0, 10-1 do
        x[k+1] = k/10.0 * 4*3.14 + 0.01;
        y[k+1] = 7*math.sin(x[k+1])/x[k+1] + (#shapes-i)*5;
      end

      plot:lastGraph():setVector(x, y);
      plot:lastGraph():rescaleAxes(true);
      plot:lastGraph():setPen(pen);
      plot:lastGraph():setName(pa.getTableIndexByValue(qcp.ScatterStyle, shapes[i+1]));
      plot:lastGraph():setLineStyle(qcp.Graph.lsLine);

      -- set scatter style:
      if (shapes[i+1] ~= qcp.ScatterStyle.ssCustom) then
        plot:lastGraph():setScatterStyle(luaplot.ScatterStyleConstructor.fromShapeAndSize(shapes[i+1], 10));
      else
        local customScatterPath = luaplot.QPainterPath();
        for i=0, 3-1 do
          customScatterPath:cubicToXY(math.cos(2*math.pi*i/3.0)*9, math.sin(2*math.pi*i/3.0)*9, math.cos(2*math.pi*(i+0.9)/3.0)*9, math.sin(2*math.pi*(i+0.9)/3.0)*9, 0, 0);
        end
        plot:lastGraph():setScatterStyle(
          luaplot.ScatterStyleConstructor.fromPainterPath(
            customScatterPath,
            luaplot.PenConstructor.fromBrush(luaplot.BrushConstructor.fromColor(luaplot.ColorConstructor.fromGlobal(qt.black), qt.SolidPattern), 0, qt.SolidLine, qt.SquareCap, qt.BevelJoin),
            luaplot.BrushConstructor.fromColor(luaplot.ColorConstructor.fromRGB(40, 70, 255, 50), qt.SolidPattern),
            10
          )
        );
      end
    end

    -- set blank axis lines:
    plot:rescaleAxes();
    plot.xAxis:setTicks(false);
    plot.yAxis:setTicks(false);
    plot.xAxis:setTickLabels(false);
    plot.yAxis:setTickLabels(false);
    -- make top right axes clones of bottom left axes:
    plot:axisRect(0):setupFullAxesBox();
end

function fw(w)
  local p = w:getPlot()
  fp(p)
end

qcp.startMainWindow(fw)
