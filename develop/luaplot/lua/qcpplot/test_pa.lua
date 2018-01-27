local pa = require("print_any")

--pa.print_any(_G)
pa.print_any(_G["print_any"])
pa.print_any(print_any)
pa.print_any(_G["os"])
pa.print_any(os)

print_any.print_any(table)

---[[
local t1 = {"v1", "v2", v3="v3", c="ccc", d=111}
local t2 = {false, t1}
t2[3]=t2
t1.a=t1
t1.b=t2

--print_any("_G=", _G)
pa.print_any(t1[1])
pa.print_any(t1)
pa.print_any(t2)
pa.print_any(getmetatable(t1))
--print_t(t1); output("\n")
--print(t2)
--print_t(t2); output("\n")
--print_t(t); output("\n")

--print_t(_G)
--print_t(print_t); output("\n")
--]]