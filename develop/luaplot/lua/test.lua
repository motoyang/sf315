
local pa=require("qcpplot.print_any")
local qcp=require("qcpplot.qcp")
local qt=require("qcpplot.qt5")

local StatisticalBoxData = {
  key = 1.1,
  minimum = 2.2,
  lowerQuartile = 3.3,
  median = 4.4,
  upperQuartile = 5.5,
  maximum = 6.6,
};
StatisticalBoxData.outliers = {11.1, 22.2, 33.3, 44.4, 55.5}
pa.pprint("StatisticalBoxData=", StatisticalBoxData)

local a = luaplot.getStatisticalBoxData(StatisticalBoxData)
pa.pprint("a=", a)
--]]
