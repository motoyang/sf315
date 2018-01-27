local ar = require("array")
local pa = require("print_any")

pa.print_any(ar)

--[[
local a1 = ar.linespace(1, 10, 10, false)
pa.print_any(a1)

local a2 = ar.linespace(1, 10, 10)
pa.print_any(a2)

local a3 = ar.linespace(1, 10, 10, true)
pa.print_any(a3)

local b1 = ar.logspace(1, 2, 10, false)
pa.print_any(b1)

local b2 = ar.logspace(1, 3, 10, true, 2)
pa.print_any(b2)

local b3 = ar.logspace(1, 4, 10, true, math.pi)
pa.print_any(b3)
--]]

local c1 = ar.arange(1, 10, 1)
pa.print_any(c1)
ar.for_each(c1, function (v) return v*2 end)
pa.print_any(c1)

local d1 = ar.transform(c1, function(v) return v/3 end)
pa.print_any(d1)

local c2 = ar.arange(1, 10, 0.031)
--pa.print_any(c2)
