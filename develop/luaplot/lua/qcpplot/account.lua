-- print_any.lua

-- 定义了print_any模块
local modname = ...
local M = {}

-- return modname的功能
_G[modname]=M
package.loaded[modname] = M  

function M.linespace(start, stop, count, end_point)
  local end_point = end_point or true

  if (start < stop or count < 2) then error("parameter error", 1) end

  local r = {}
  local step = (stop - start) / ((end_point and (count - 1)) or count)
  for v = start, stop, step do r[#r] = v end

  return r
end

function M.logspace(start, stop, count, end_point, base)
  local end_point = end_point or true
  local base = base or 10.0
end

function M.arange(start, stop, step)
  local end_point = end_point or true
  local base = base or 10.0
end


M.Account = {
  balance = 0,
  new = function(self, o)
      o = o or {}
      self.__index = self
      setmetatable(o, self)
      return o
    end,
  withdraw = function(self, v) self.balance = self.balance - v end,
  deposit = function(self, v) self.balance = self.balance + v end,
}

M.SpecialAccount = M.Account:new{limit=1000}

function M.SpecialAccount:withdraw(v)
  if (v - self.balance >= self:getLimit()) then print("insufficient funds")
  else self.balance = self.balance - v end
end

function M.SpecialAccount:getLimit()
  return self.limit or 0
end

M.Sp2 = M.SpecialAccount:new{money=2222}

function M.Sp2:getMoney() return self.money - self.limit end


-- 返回本模块的table
return M

