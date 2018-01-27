-- print_any.lua

-- 定义了print_any模块
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
    output("(", o_string, ")") 
    if not t[o_string] then 
      t[o_string]=true
      output("{")
      for k, v in pairs(o) do
        print_t(k, t)
        output("=")
        print_t(v, t)
        output(", ")
      end
      output("}")
      t[o_string]=nil
    end
  else error("can't output a " .. o_type) end
end

-- 本模块对外提供的函数
function M.print_any(value)
  print_t(value)
  output("\n")
end

-- 返回本模块的table
return M



