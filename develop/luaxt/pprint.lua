-- pprint.lua

-- 定义模块
local modname = ...
local M = {}

-- return modname的功能
_G[modname]=M
package.loaded[modname] = M

-- 私有函数的定义

local function output(...)
  io.stdout:write(...)
end

local function print_t(o, t)
  local t = t or {}

  local o_type = type(o)
  local o_string = tostring(o)

  if o_type == "nil" then output("nil")
  elseif o_type == "number" then output(o)
  elseif o_type == "string" then output(string.format("%q", o))
  elseif o_type == "boolean" then output(o_string)
  elseif o_type == "function" then output(o_string)
  elseif o_type == "thread" then output(o_string)
  elseif o_type == "userdata" then output(o_string)
  elseif o_type == "table" then
--    output("(", o_string, ")")
    if not t[o_string] then
      t[o_string]=true
      output("{")
      local first = true
      for k, v in pairs(o) do
        if not first then
          output(", ")
        else
          first = not first
        end
        output(k.."=")
        print_t(v, t)
      end
      output("}")
      t[o_string]=nil
    end
  else error("can't output a " .. o_type) end
end

-- 本模块对外提供的函数

function M.print(...)
  for i = 1, select("#", ...) do
    local v = select(i, ...)
    if type(v) == "string" then
      output(v)
    else
      print_t(v)
    end
  end
  output("\n")
end

function M.getTableIndexByValue(t, value)
  local r = {}
  for k, v in pairs(t) do
    r[v] = k
  end
  return r[value]
end

function M.demo()
  local t1 = {[0]="abcde", 1, 3.3, {['a']=1, ['b']=100, ['c']=10000}, {"ab", "bc", "ca"}}
  M.print(t1)
end

-- 返回本模块的table
return M



