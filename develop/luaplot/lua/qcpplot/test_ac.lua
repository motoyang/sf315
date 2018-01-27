local ac = require("account")
local pa = require("print_any")

io.write("ac="); pa.print_any(ac)

io.write("ac.Account="); pa.print_any(ac.Account)
io.write("ac.Account.mt="); pa.print_any(getmetatable(ac.Account))

local a =ac.Account:new{balance=0}
io.write("a="); pa.print_any(a)
io.write("a.mt="); pa.print_any(getmetatable(a))

a:deposit(100)
io.write("a="); pa.print_any(a)

local s = ac.SpecialAccount:new()
s:withdraw(238)
io.write("s="); pa.print_any(s)
io.write("s.mt="); pa.print_any(getmetatable(s))
io.write("s.mt.mt="); pa.print_any(getmetatable(getmetatable(s)))

s:withdraw(838)
io.write("s="); pa.print_any(s)

s:deposit(200)
s:withdraw(838)
io.write("s="); pa.print_any(s)

local s2=ac.Sp2:new()
print(s2:getMoney())
io.write("s2="); pa.print_any(s2)

local s22=ac.Sp2:new{money=3333, limit=1100, balance=203}
print(s22:getMoney())
io.write("s22="); pa.print_any(s22)
