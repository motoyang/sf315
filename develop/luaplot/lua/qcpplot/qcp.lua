-- qcp.lua

-- 定义了本模块
local modname = ...

local M = {
    -- enum ResolutionUnit {
    ruDotsPerMeter=0       -- Resolution is given in dots per meter (dpm)
    ,ruDotsPerCentimeter=1 -- Resolution is given in dots per centimeter (dpcm)
    ,ruDotsPerInch=2       -- Resolution is given in dots per inch (DPI/PPI)
    ,
--  };
    
    -- enum ExportPen {
    epNoCosmetic=0     -- Cosmetic pens are converted to pens with pixel width 1 when exporting
    ,epAllowCosmetic=1 -- Cosmetic pens are exported normally (e.g. in PDF exports, cosmetic pens always appear as 1 pixel on screen, independent of viewer zoom level)
    ,
--  }

    -- enum SignDomain {
    sdNegative=0  -- The negative sign domain, i.e. numbers smaller than    
    ,sdBoth=1     -- Both sign domains, including zero, i.e. all numbers
    ,sdPositive=2 -- The positive sign domain, i.e. numbers greater than 
    ,
--  };

    -- enum MarginSide {
    msLeft     = 0x01 -- <tt>0x01</tt> left margin
    ,msRight   = 0x02 -- <tt>0x02</tt> right margin
    ,msTop     = 0x04 -- <tt>0x04</tt> top margin
    ,msBottom  = 0x08 -- <tt>0x08</tt> bottom margin
    ,msAll     = 0xFF -- <tt>0xFF</tt> all margins
    ,msNone    = 0x00 -- <tt>0x00</tt> no margin
    ,
--  };
-- Q_DECLARE_FLAGS(MarginSides, MarginSide)

    -- enum AntialiasedElement {
    aeAxes           = 0x0001 -- <tt>0x0001</tt> Axis base line and tick marks
    ,aeGrid          = 0x0002 -- <tt>0x0002</tt> Grid lines
    ,aeSubGrid       = 0x0004 -- <tt>0x0004</tt> Sub grid lines
    ,aeLegend        = 0x0008 -- <tt>0x0008</tt> Legend box
    ,aeLegendItems   = 0x0010 -- <tt>0x0010</tt> Legend items
    ,aePlottables    = 0x0020 -- <tt>0x0020</tt> Main lines of plottables
    ,aeItems         = 0x0040 -- <tt>0x0040</tt> Main lines of items
    ,aeScatters      = 0x0080 -- <tt>0x0080</tt> Scatter symbols of plottables (excluding scatter symbols of type ssPixmap)
    ,aeFills         = 0x0100 -- <tt>0x0100</tt> Borders of fills (e.g. under or between graphs)
    ,aeZeroLine      = 0x0200 -- <tt>0x0200</tt> Zero-lines, see  QCPGrid::setZeroLinePen
    ,aeOther         = 0x8000 -- <tt>0x8000</tt> Other elements that don't fit into any of the existing categories
    ,aeAll           = 0xFFFF -- <tt>0xFFFF</tt> All elements
    ,aeNone          = 0x0000 -- <tt>0x0000</tt> No elements
    ,
--  }
-- Q_DECLARE_FLAGS(AntialiasedElements, AntialiasedElement)

    -- enum PlottingHint {
    phNone              = 0x000 -- <tt>0x000</tt> No hints are    
    ,phFastPolylines    = 0x001 -- <tt>0x001</tt> Graph/Curve lines are drawn with a faster method. This reduces the quality especially of the line segment joins, thus is most effective for pen sizes larger than 1. It is only used for solid line pens.
    ,phImmediateRefresh = 0x002 -- <tt>0x002</tt> causes an immediate repaint() instead of a soft update() when QCustomPlot::replot() is called with parameter  QCustomPlot::rpRefreshHint. This is set by default to prevent the plot from freezing on fast consecutive replots (e.g. user drags ranges with mouse).
    ,phCacheLabels      = 0x004 -- <tt>0x004</tt> axis (tick) labels will be cached as pixmaps, increasing replot performance.
    ,
--   };
-- Q_DECLARE_FLAGS(PlottingHints, PlottingHint)

    -- enum Interaction {
    iRangeDrag         = 0x001 -- <tt>0x001</tt> Axis ranges are draggable (see  QCPAxisRect::setRangeDrag,  QCPAxisRect::setRangeDragAxes)
    ,iRangeZoom        = 0x002 -- <tt>0x002</tt> Axis ranges are zoomable with the mouse wheel (see  QCPAxisRect::setRangeZoom,  QCPAxisRect::setRangeZoomAxes)
    ,iMultiSelect      = 0x004 -- <tt>0x004</tt> The user can select multiple objects by holding the modifier set by  QCustomPlot::setMultiSelectModifier while clicking
    ,iSelectPlottables = 0x008 -- <tt>0x008</tt> Plottables are selectable (e.g. graphs, curves, bars,... see QCPAbstractPlottable)
    ,iSelectAxes       = 0x010 -- <tt>0x010</tt> Axes are selectable (or parts of them, see QCPAxis::setSelectableParts)
    ,iSelectLegend     = 0x020 -- <tt>0x020</tt> Legends are selectable (or their child items, see QCPLegend::setSelectableParts)
    ,iSelectItems      = 0x040 -- <tt>0x040</tt> Items are selectable (Rectangles, Arrows, Textitems, etc. see  QCPAbstractItem)
    ,iSelectOther      = 0x080 -- <tt>0x080</tt> All other objects are selectable (e.g. your own derived layerables, other layout elements,...)
    ,
--  };
-- Q_DECLARE_FLAGS(Interactions, Interaction)

    -- enum SelectionRectMode { 
    srmNone=0    -- The selection rect is disabled, and all mouse events are forwarded to the underlying objects, e.g. for axis range dragging
    ,srmZoom=1   -- When dragging the mouse, a selection rect becomes active. Upon releasing, the axes that are currently set as range zoom axes ( QCPAxisRect::setRangeZoomAxes) will have their ranges zoomed accordingly.
    ,srmSelect=2 -- When dragging the mouse, a selection rect becomes active. Upon releasing, plottable data points that were within the selection rect are selected, if the plottable's selectability setting permits. (See   dataselection "data selection mechanism" for details.)
    ,srmCustom=3 -- When dragging the mouse, a selection rect becomes active. It is the programmer's responsibility to connect according slots to the selection rect's signals (e.g.  QCPSelectionRect::accepted) in order to process the user interaction.
    ,
--  };

    -- enum SelectionType {
    stNone=0                -- The plottable is not selectable
    ,stWhole=1              -- Selection behaves like  stMultipleDataRanges, but if there are any data points selected, the entire plottable is drawn as selected.
    ,stSingleData=2         -- One individual data point can be selected at a time
    ,stDataRange=3          -- Multiple contiguous data points (a data range) can be selected
    ,stMultipleDataRanges=4 -- Any combination of data points/ranges can be selected
    ,
--  };
  } -- the end of local M

M.Layer = {
    -- enum LayerMode { 
    lmLogical=0   -- Layer is used only for rendering order, and shares paint buffer with all other adjacent logical layers.
    ,lmBuffered=1 -- Layer has its own paint buffer and may be replotted individually (see  replot).
    ,
--  };
  }

M.LayoutGrid = {
    -- enum FillOrder { 
    foRowsFirst=0    -- Rows are filled first, and a new element is wrapped to the next column if the row count would exceed  setWrap.
    ,foColumnsFirst=1 -- Columns are filled first, and a new element is wrapped to the next row if the column count would exceed  setWrap.
    ,
--  };
  }

M.LayoutInset = {
    -- enum InsetPlacement {
    ipFree=0            -- The element may be positioned/sized arbitrarily, see  setInsetRect
    ,ipBorderAligned=1  -- The element is aligned to one of the layout sides, see  setInsetAlignment
    ,
--  };
  }

M.LineEnding = {
    -- enum EndingStyle {
    esNone=0          -- No ending decoration
    ,esFlatArrow=1    -- A filled arrow head with a straight/flat back (a triangle)
    ,esSpikeArrow=2   -- A filled arrow head with an indented back
    ,esLineArrow=3    -- A non-filled arrow head with open back
    ,esDisc=4         -- A filled circle
    ,esSquare=5       -- A filled square
    ,esDiamond=6      -- A filled diamond (45 degrees rotated square)
    ,esBar=7          -- A bar perpendicular to the line
    ,esHalfBar=8      -- A bar perpendicular to the line, pointing out to only one side (to which side can be changed with  setInverted)
    ,esSkewedBar=9    -- A bar that is skewed (skew controllable via  setLength)
    ,
--  };
  }

M.AxisTicker = {
    -- enum TickStepStrategy {
    tssReadability=0    -- A nicely readable tick step is prioritized over matching the requested number of ticks (see  setTickCount)
    ,tssMeetTickCount=1 -- Less readable tick steps are allowed which in turn facilitates getting closer to the requested tick count
    ,
--  };
  }

M.AxisTickerDateTime = {
    -- enum DateStrategy {
    dsNone=0, dsUniformTimeInDay=1, dsUniformDayInMonth=2
    ,
--  };
  }

M.AxisTickerTime = {
    -- enum TimeUnit {
    tuMilliseconds=0 -- Milliseconds, one thousandth of a second (%%z in  setTimeFormat)
    ,tuSeconds=1     -- Seconds (%%s in  setTimeFormat)
    ,tuMinutes=2     -- Minutes (%%m in  setTimeFormat)
    ,tuHours=3       -- Hours (%%h in  setTimeFormat)
    ,tuDays=4        -- Days (%%d in  setTimeFormat)
    ,
--  };
  }

M.AxisTickerFixed = {
    -- enum ScaleStrategy {
    ssNone=0      -- Modifications are not allowed, the specified tick step is absolutely fixed. This might cause a high tick density and overlapping=1 labels if the axis range is zoomed out.
    ,ssMultiples=1 -- An integer multiple of the specified tick step is allowed. The used factor follows the base class properties of  setTickStepStrategy and  setTickCount.
    ,ssPowers=2    -- An integer power of the specified tick step is allowed.
    ,
--  };
  }

M.AxisTickerPi = {
    -- FractionStyle { 
    fsFloatingPoint=0     -- Fractions are displayed as regular decimal floating point numbers, e.g. "0.25" or "0.125".
    ,fsAsciiFractions=1   -- Fractions are written as rationals using ASCII characters only, e.g. "1/4" or "1/8"
    ,fsUnicodeFractions=2 -- Fractions are written using sub- and superscript UTF-8 digits and the fraction symbol.
    ,
--  };
  }

M.Axis = {
    -- enum AxisType {
    atLeft    = 0x01  -- <tt>0x01</tt> Axis is vertical and on the left side of the axis rect
    ,atRight  = 0x02  -- <tt>0x02</tt> Axis is vertical and on the right side of the axis rect
    ,atTop    = 0x04  -- <tt>0x04</tt> Axis is horizontal and on the top side of the axis rect
    ,atBottom = 0x08  -- <tt>0x08</tt> Axis is horizontal and on the bottom side of the axis rect
    ,
--  };
-- Q_FLAGS(AxisTypes)

    -- enum LabelSide {
    lsInside=0    -- Tick labels will be displayed inside the axis rect and clipped to the inner axis rect
    ,lsOutside=1  -- Tick labels will be displayed outside the axis rect
    ,
--  };

    -- enum ScaleType {
    stLinear=0       -- Linear scaling
    ,stLogarithmic=1 -- Logarithmic scaling with correspondingly transformed axis coordinates (possibly also  setTicker to a  QCPAxisTickerLog instance).
    ,
--  };

    -- enum SelectablePart {
    spNone        = 0      -- None of the selectable parts
    ,spAxis       = 0x001  -- The axis backbone and tick marks
    ,spTickLabels = 0x002  -- Tick labels (numbers) of this axis (as a whole, not individually)
    ,spAxisLabel  = 0x004  -- The axis label
    ,
--  };
-- Q_FLAGS(SelectableParts)
  }

M.ScatterStyle = {
    -- enum ScatterProperty {
    spNone  = 0x00  -- <tt>0x00</tt> None
    ,spPen   = 0x01  -- <tt>0x01</tt> The pen property, see  setPen
    ,spBrush = 0x02  -- <tt>0x02</tt> The brush property, see  setBrush
    ,spSize  = 0x04  -- <tt>0x04</tt> The size property, see  setSize
    ,spShape = 0x08  -- <tt>0x08</tt> The shape property, see  setShape
    ,spAll   = 0xFF  -- <tt>0xFF</tt> All properties
    ,
--  };
-- Q_FLAGS(ScatterProperties)

    -- enum ScatterShape {
    ssNone=0       -- no scatter symbols are drawn (e.g. in QCPGraph, data only represented with lines)
    ,ssDot=1       -- {ssDot.png} a single pixel (use  ssDisc or  ssCircle if you want a round shape with a certain radius)=2
    ,ssCross=2     -- {ssCross.png} a cross
    ,ssPlus=3      -- {ssPlus.png} a plus
    ,ssCircle=4    -- {ssCircle.png} a circle
    ,ssDisc=5      -- {ssDisc.png} a circle which is filled with the pen's color (not the brush as with ssCircle)
    ,ssSquare=6    -- {ssSquare.png} a square
    ,ssDiamond=7   -- {ssDiamond.png} a diamond
    ,ssStar=8      -- {ssStar.png} a star with eight arms, i.e. a combination of cross and plus
    ,ssTriangle=9  -- {ssTriangle.png} an equilateral triangle, standing on baseline
    ,ssTriangleInverted=10 -- {ssTriangleInverted.png} an equilateral triangle, standing on corner
    ,ssCrossSquare=11      -- {ssCrossSquare.png} a square with a cross inside
    ,ssPlusSquare=12       -- {ssPlusSquare.png} a square with a plus inside
    ,ssCrossCircle=13      -- {ssCrossCircle.png} a circle with a cross inside
    ,ssPlusCircle=14       -- {ssPlusCircle.png} a circle with a plus inside
    ,ssPeace=15     -- {ssPeace.png} a circle, with one vertical and two downward diagonal lines
    ,ssPixmap=16    -- a custom pixmap specified by  setPixmap, centered on the data point coordinates
    ,ssCustom=17    -- custom painter operations are performed per scatter (As QPainterPath, see  setCustomPath)
    ,
--  };
  }

M.ItemPosition = {
    -- PositionType {
    ptAbsolute=0        -- Static positioning in pixels, starting from the top left corner of the viewport/widget.
    ,ptViewportRatio=1  -- Static positioning given by a fraction of the viewport size. For example, if you call setCoords(0, 0), the position will be at the top left corner of the viewport/widget. setCoords(1, 1) will be at the bottom right corner, setCoords(0.5, 0) will be horizontally centered and vertically at the top of the viewport/widget, etc.
    ,ptAxisRectRatio=2  -- Static positioning given by a fraction of the axis rect size (see  setAxisRect). For example, if you call setCoords(0, 0), the position will be at the top left corner of the axis rect. setCoords(1, 1) will be at the bottom right corner, setCoords(0.5, 0) will be horizontally centered and vertically at the top of the axis rect, etc. You can also go beyond the axis rect by providing negative coordinates or coordinates larger than 1.
    ,ptPlotCoords=3     -- Dynamic positioning at a plot coordinate defined by two axes (see  setAxes).
    ,
--  };
  }

M.CustomPlot = {
    -- LayerInsertMode {
    limBelow=0  -- Layer is inserted below other layer
    ,limAbove=1 -- Layer is inserted above other layer
    ,
--  };

    -- enum RefreshPriority {
    rpImmediateRefresh=0 -- Replots immediately and repaints the widget immediately by calling QWidget::repaint() after the replot
    ,rpQueuedRefresh=1   -- Replots immediately, but queues the widget repaint, by calling QWidget::update() after the replot. This way multiple redundant widget repaints can be avoided.
    ,rpRefreshHint=2     -- Whether to use immediate or queued refresh depends on whether the plotting hint  QCP::phImmediateRefresh is set, see  setPlottingHints.
    ,rpQueuedReplot=3    -- Queues the entire replot for the next event loop iteration. This way multiple redundant replots can be avoided. The actual replot is then done with  rpRefreshHint priority.
    ,
--  };
  }

M.ColorGradient = {
    -- ColorInterpolation {
    ciRGB=0  -- Color channels red, green and blue are linearly interpolated
    ,ciHSV=1 -- Color channels hue, saturation and value are linearly interpolated (The hue is interpolated over the shortest angle distance)
    ,
--  };

    -- enum GradientPreset {
    gpGrayscale=0  -- Continuous lightness from black to white (suited for non-biased data representation)
    ,gpHot=1       -- Continuous lightness from black over firey colors to white (suited for non-biased data representation)
    ,gpCold=2      -- Continuous lightness from black over icey colors to white (suited for non-biased data representation)
    ,gpNight=3     -- Continuous lightness from black over weak blueish colors to white (suited for non-biased data representation)
    ,gpCandy=4     -- Blue over pink to white
    ,gpGeography=5 -- Colors suitable to represent different elevations on geographical maps
    ,gpIon=6       -- Half hue spectrum from black over purple to blue and finally green (creates banding illusion but allows more precise magnitude estimates)
    ,gpThermal=7   -- Colors suitable for thermal imaging, ranging from dark blue over purple to orange, yellow and white
    ,gpPolar=8     -- Colors suitable to emphasize polarity around the center, with blue for negative, black in the middle and red for positive values=9
    ,gpSpectrum=9  -- An approximation of the visible light spectrum (creates banding illusion but allows more precise magnitude estimates)
    ,gpJet=10       -- Hue variation similar to a spectrum, often used in numerical visualization (creates banding illusion but allows more precise magnitude estimates)
    ,gpHues=11      -- Full hue cycle, with highest and lowest color red (suitable for periodic data, such as angles and phases, see  setPeriodic)
    ,
--  };
  }

M.SelectionDecoratorBracket = {
    -- enum BracketStyle {
    bsSquareBracket=0 -- A square bracket is drawn.
    ,bsHalfEllipse=1   -- A half ellipse is drawn. The size of the ellipse is given by the bracket width/height properties.
    ,bsEllipse=2       -- An ellipse is drawn. The size of the ellipse is given by the bracket width/height properties.
    ,bsPlus=3         -- A plus is drawn.
    ,bsUserStyle=4    -- Start custom bracket styles at this index when subclassing and reimplementing  drawBracket.
    ,
--  };
  }

M.Legend = {
    -- enum SelectablePart {
    spNone        = 0x000 -- <tt>0x000</tt> None
    ,spLegendBox  = 0x001 -- <tt>0x001</tt> The legend box (frame)
    ,spItems      = 0x002 -- <tt>0x002</tt> Legend items individually (see  selectedItems)
    ,
--  };
-- Q_FLAGS(SelectableParts)
  }
M.Graph = {
    -- enum LineStyle {
    lsNone=0        -- data points are not connected with any lines (e.g. data only represented with symbols according to the scatter style, see  setScatterStyle)
    ,lsLine=1       -- data points are connected by a straight line
    ,lsStepLeft=2   -- line is drawn as steps where the step height is the value of the left data point
    ,lsStepRight=3  -- line is drawn as steps where the step height is the value of the right data point
    ,lsStepCenter=4 -- line is drawn as steps where the step is in between two data points
    ,lsImpulse=5    -- each data point is represented by a line parallel to the value axis, which reaches from the data point to the zero-value-line
    ,
--  };
  }

M.Curve = {
    -- enum LineStyle {
    lsNone=0  -- No line is drawn between data points (e.g. only scatters)
    ,lsLine=1 -- Data points are connected with a straight line
    ,
--  };
  }

M.BarsGroup = {
    -- enum SpacingType {
    stAbsolute=0       -- Bar spacing is in absolute pixels
    ,stAxisRectRatio=1 -- Bar spacing is given by a fraction of the axis rect size
    ,stPlotCoords=2    -- Bar spacing is in key coordinates and thus scales with the key axis range
    ,
--  };
  }

M.Bars = {
    -- enum WidthType {
    wtAbsolute=0       -- Bar width is in absolute pixels
    ,wtAxisRectRatio=1 -- Bar width is given by a fraction of the axis rect size
    ,wtPlotCoords=2    -- Bar width is in key coordinates and thus scales with the key axis range
    ,
--  };
  }

M.Financial = {
    -- enum WidthType {
    wtAbsolute=0       -- width is in absolute pixels
    ,wtAxisRectRatio=1 -- width is given by a fraction of the axis rect size
    ,wtPlotCoords=2    -- width is in key coordinates and thus scales with the key axis range
    ,
--  };

    -- enum ChartStyle {
    csOhlc=0         -- Open-High-Low-Close bar representation
    ,csCandlestick=1  -- Candlestick representation
    ,
--  };
  }

M.ErrorBars = {
    -- enum ErrorType {
    etKeyError=0    -- The errors are for the key dimension (bars appear parallel to the key axis)
    ,etValueError=1 -- The errors are for the value dimension (bars appear parallel to the value axis)
    ,
--  };
  }

M.ItemTracer = {
    -- enum TracerStyle {
    tsNone=0        -- The tracer is not visible
    ,tsPlus=1       -- A plus shaped crosshair with limited size
    ,tsCrosshair=2  -- A plus shaped crosshair which spans the complete axis rect
    ,tsCircle=3     -- A circle
    ,tsSquare=4     -- A square
    ,
--  };
  }

M.ItemBracket = {
    -- enum BracketStyle {
    bsSquare=0  -- A brace with angled edges
    ,bsRound=1  -- A brace with round edges
    ,bsCurly=2  -- A curly brace
    ,bsCalligraphic=3 -- A curly brace with varying stroke width giving a calligraphic impression
    ,
--  };
  } 

--------------------

local function initPlot(p, tfName, t, interaction)
  if tfName then 
    if t == nil then t = 0 end
    p:setTimer(tfName, t)
  end

  local e = p:plotLayout():elementCount()
  if (interaction and (e == 1)) then
    p:setInteractions(M.iRangeDrag + M.iRangeZoom)
  end
end

--------------------

function M.startPlot(f, tfName, t)
  local p = luaplot.LuaPlot(nil)
  p:resize(800, 600)
  p:setWindowTitle("luaplot")
  p:setLuaState()

  -- 执行用户提供的plot函数
  f(p)
  initPlot(p, tfName, t)

  p:show()
  luaplot.App.exec()
end

--------------------

function M.startMainWindow(f, tfName, t)
  local w = luaplot.MainWindow(nil)
  w:resize(800, 600)
  w:setWindowTitle("luaplot")

  local p = w:getPlot()
  p:setLuaState()
  -- 执行用户提供的mainWindow函数
  f(w)
  initPlot(p, tfName, t, true)

  w:show()
  luaplot.App.exec()
end

--------------------

M.Expression = {
  name = "name of the expression",
  expression = "mathematic expression",
  xLower = -10, xUpper = 10,
  yLower = -10, yUpper = 10,
  pointsOfWidth = 800, pointsOfHeight = 600,
  splitInPoint = 3,
};
  
function M.Expression:new(e)
  local r = {}
  for k, v in pairs(self) do
    r[k] = v
  end
  for k, v in pairs(e) do
    r[k] = v
  end
  return r
end

function M.Expression:calcDefaultDiff()
  local dx = (self.xUpper - self.xLower) / self.pointsOfWidth;
  local dy = (self.yUpper - self.yLower) / self.pointsOfHeight;

  return math.sqrt(dx*dx + dy*dy)
end

function M.addExpression(p, e)
  local function f(x, y)
    return e:f(x, y)
  end
  -- 临时生成一个字符串，作为从c/c++调用lua function的名字，
  -- 调用完成后，马上删除该全局变量。
  local ef_name = {}
  e.luaFunctionName = tostring(ef_name)

  _G[e.luaFunctionName] = f
  local curve = p:addLuaExpression(e)
  _G[e.luaFunctionName] = nil

  curve:setScatterStyle(luaplot.ScatterStyleConstructor.fromShapeAndSize(M.ScatterStyle.ssSquare, 1))
  return curve
end

function M.startExpression(e)
  local function fp(p)
    local curve = M.addExpression(p, e)
    p:setWindowTitle(e.name .. ", " .. e.expression)
  end

  M.startPlot(fp)
end

--------------------

-- return modname的功能
_G[modname]=M
package.loaded[modname] = M  

