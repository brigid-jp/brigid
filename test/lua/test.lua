-- Copyright (c) 2019 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local build = ...

if build == "autotools" then
  package.cpath = [[../../src/lua/.libs/?.so;;]]
elseif build == "windows" then
  package.cpath = [[..\..\?.dll;;]]
end

local total = 0;
local pass = 0;
local fail = 0;

for file in io.lines() do
  total = total + 1
  local result, message = pcall(assert(loadfile(file)))
  if result then
    pass = pass + 1
    io.write("PASS ", file, "\n")
  else
    fail = fail + 1
    io.write("FAIL ", file, " ", message, "\n")
  end
end

io.write(([[
============================================================
TOTAL: %d
PASS:  %d
FAIL:  %d
============================================================
]]):format(total, pass, fail))

if fail == 0 then
  os.exit(0)
else
  os.exit(1)
end
