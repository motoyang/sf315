-- hex.lua

-- 定义模块
local modname = ...
local M = {}

-- return modname的功能
_G[modname]=M
package.loaded[modname] = M

-- 私有函数的定义

local h2b = {
  ["0"] = 0,
  ["1"] = 1,
  ["2"] = 2,
  ["3"] = 3,
  ["4"] = 4,
  ["5"] = 5,
  ["6"] = 6,
  ["7"] = 7,
  ["8"] = 8,
  ["9"] = 9,
  ["A"] = 10,
  ["B"] = 11,
  ["C"] = 12,
  ["D"] = 13,
  ["E"] = 14,
  ["F"] = 15
}

-- 本模块对外提供的函数

function M.decode(hexstr)
  return string.gsub(hexstr, "(%x)(%x)", function (h, l)
    return string.char(h2b[h] * 16 + h2b[l]) end)
end

function M.encode(s)
  return string.gsub(s, "(.)", function (x)
    return string.format("%02X", string.byte(x)) end)
end

function M.print(s)
  local i = 0
  x = s:gsub("(%x%x%x%x%x%x%x%x)", "%1 ")
    :gsub("(%w+ %w+ %w+ %w+ )", function(line)
      i = i + 1
      return string.format("%s\n%04X: ", line, i * 16) end
  )
  print("0000: " .. x)
end

function M.demo()
  hexStr = "05879AAF9A65F3332A3B5E9AAFAF65F33365F3332A39AAF65F333B5E678799AAF65F333AAF3B5E67879AA9AAF65F333F65F3332A3B5E9AAF65F33367879AAF65F333"
  print(hexStr)

  local binVal = M.decode(hexStr)
  local s = M.encode(binVal)
  print(s)
  M.print(s)

  s = M.decode("050607")
  -- print(s)
  s = M.encode(s)
  print(s)
  M.print(s)

  s = M.encode("\x08\x09\x0a\x00\xaa\xbb\xcc\xdd")
  M.print(s)

  local b = M.decode(s)
  local s2 = M.encode(b)
  M.print(s2)

  M.print(M.encode(string.dump(M.encode, true)))
end

-- 返回本模块的table
return M



