-- Copyright (c) 2021 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

package.cpath = ".libs/?.so;;"
local module = require "test_address_space"

print("sizeof_void_pointer", module.sizeof_void_pointer)

local v = 1
for i = 0, 32 do
  local u = v - 1
  local a = u % 256 u = (u - a) / 256
  local b = u % 256 u = (u - b) / 256
  local c = u % 256 u = (u - c) / 256
  local d = u
  local s = string.char(0xFF, 0xFF, 0xFF, 0xFF, a, b, c, d)
  print(("string_to_lightuserdata(%2dbit)"):format(i + 32), module.string_to_lightuserdata(s))
  v = v * 2
end

-- local str47 = "\255\255\255\255\255\127\000\000"
-- local str48 = "\255\255\255\255\255\255\000\000"
--
-- print("string_to_lightuserdata(str47)", module.string_to_lightuserdata(str47))
-- print("string_to_lightuserdata(str48)", module.string_to_lightuserdata(str48))
