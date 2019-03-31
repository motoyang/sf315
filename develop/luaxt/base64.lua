-- base64.lua

-- 定义模块
local modname = ...
local M = {}

-- return modname的功能
_G[modname] = M
package.loaded[modname] = M

-- 私有函数的定义

local bs = { [0] =
   'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P',
   'Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f',
   'g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v',
   'w','x','y','z','0','1','2','3','4','5','6','7','8','9','+','/',
}

local ds = {}
for i = 0, 63 do
  ds[bs[i]] = i
end

-- 本模块对外提供的函数

--[[
          a                   b                   c
  +-----------------+-----------------+-----------------+
  | 0 0 0 0 0 0 1 1 | 1 1 1 1 2 2 2 2 | 2 2 3 3 3 3 3 3 |
  +|- - - - - -|- - + - - - -|- - - - + - -|- - - - - -|+
  /           /              |              \           \
 /           /               |               \           \
    a>>2     (a&3)<<4|b>>4    (b&15)<<2|c>>6      c&63
]]
function M.encode(s)
  local byte, rep = string.byte, string.rep
  local pad = 2 - ((#s - 1) % 3)
  s =
    (s .. rep("\0", pad)):gsub(
    "...",
    function(cs)
      local a, b, c = byte(cs, 1, 3)
      return bs[a >> 2] .. bs[(a & 3) << 4 | b >> 4] .. bs[(b & 15) << 2 | c >> 6] .. bs[c & 63]
    end
  )
  return s:sub(1, #s - pad) .. rep("=", pad)
end

--[[
          a                   b                   c              d
  +-----------------+-----------------+-----------------+-----------------+
  | 0 0 a a a a a a | 0 0 b b b b b b | 0 0 c c c c c c | 0 0 d d d d d d |
  + - -|- - - - - - + - - - -|- - - - + - - - - - -|- - + - - - - - - - -|+
      /                      /                     |                     \
     /                      /                      |                      \
         (a<<2)|(b>>4)           (b<<4)|(c>>2)            (c<<6)|d

]]
function M.decode(s)
  local r = ""
  if s:len() % 4 > 0 then
    print("invalid base64 code.")
  else
    r = s:gsub(
      "(.)(.)(.)(.)",
      function(a, b, c, d)
        local ret_len = 3
        if c == "=" and d == "=" then
          a, b, c, d = ds[a], ds[b], 0, 0
          ret_len = 1
        elseif d == "=" then
          a, b, c, d = ds[a], ds[b], ds[c], 0
          ret_len = 2
        else
          a, b, c, d = ds[a], ds[b], ds[c], ds[d]
        end

        return string.char((a << 2) | (b >> 4), ((b & 15) << 4) | (c >> 2),
           ((c & 3) << 6) | d):sub(1, ret_len)
      end
    )
  end
  return r
end

function M.test()
  assert(M.encode("") == "")
  assert(M.encode("f") == "Zg==")
  assert(M.encode("fo") == "Zm8=")
  assert(M.encode("foo") == "Zm9v")
  assert(M.encode("foob") == "Zm9vYg==")
  assert(M.encode("fooba") == "Zm9vYmE=")
  assert(M.encode("foobar") == "Zm9vYmFy")

  assert(M.decode("") == "")
  assert(M.decode("Zg==") == "f")
  assert(M.decode("Zm8=") == "fo")
  assert(M.decode("Zm9v") == "foo")
  assert(M.decode("Zm9vYg==") == "foob")
  assert(M.decode("Zm9vYmE=") == "fooba")
  assert(M.decode("Zm9vYmFy") == "foobar")
end

function M.demo()
  local s = "将字符串 s 中,所有的(或是在 n 给出时的前 n 个) pattern (参见 §6.4.1)都替换成 repl ,并返回其副本。 repl 可以是字符串、表、或函数。 gsub 还会在第二个返回值返回一共发生了多少次匹配。"
  print("text: ".. s)

  local bcode = M.encode(s)
  print("base64: " .. bcode)

  print("decode from base64: " .. M.decode(bcode))

  print("encode to base64:")
  print(M.encode("f"))
  print(M.encode("fo"))
  print(M.encode("foo"))
  print(M.encode("foob"))
  print(M.encode("fooba"))
  print(M.encode("foobar"))

  print("decode from base64:")
  print(M.decode("Zm8="))
  print(M.decode("Zm9v"))
  print(M.decode("Zm9vYg=="))
  print(M.decode("Zm9vYmE="))
  print(M.decode("Zm9vYmFy"))
end

-- 返回本模块的table
return M
