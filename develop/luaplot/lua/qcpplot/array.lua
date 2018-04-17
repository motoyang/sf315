-- array.lua

-- 定义了本模块
local modname = ...
local M = {}

-- return modname的功能
_G[modname]=M
package.loaded[modname] = M  

function M.linspace(start, stop, count, end_point)
  -- end_point缺省值为true
  if(type(end_point)== "nil") then end_point = true end

  -- 检查输入的参数是否合适
  if ((start >= stop) or (count < 2)) then error("parameter error", 1) end

  local r = {}
  local step = (stop - start) / ((end_point and (count - 1)) or count)
  for v = start, stop, step do r[#r+1] = v end

  return r
end

function M.logspace(start, stop, count, end_point, base)
  -- end_point缺省值为true
  if(type(end_point)== "nil") then end_point = true end
  local base = base or 10.0

  -- 检查输入的参数是否合适
  if ((start >= stop) or (count < 2)) then error("parameter error", 1) end

  local r = {}
  local step = (stop - start) / ((end_point and (count - 1)) or count)
  for v = start, stop, step do r[#r+1] = base ^ v end

  return r
end

function M.arange(start, stop, step)
  local end_point = end_point or true
  local base = base or 10.0

  -- 检查输入的参数是否合适
  if ((step == 0)
      or ((step < 0) and (start <= stop))
      or ((step > 0) and (start >= stop))) 
    then error("parameter error", 1) end

  local r = {}
  for v = start, stop, step do r[#r+1] = v end

  return r
end

function M.for_each(t, f)
  for k, v in pairs(t) do t[k] = f(v) end
end

function M.transform(t, f)
  local r = {}
  for k, v in pairs(t) do r[k] = f(v) end
  return r
end

-- 返回本模块的table
return M

