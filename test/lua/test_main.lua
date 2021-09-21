-- Copyright (c) 2021 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local test_suites = require "test_suites"

local pass = 0
local fail = 0

for i = 1, #test_suites do
  local suite = test_suites[i]
  pass, fail = suite(pass, fail)
end

print(([[
============================================================
TOTAL: %d
PASS:  %d
FAIL:  %d
============================================================]]):format(pass + fail, pass, fail))

if fail == 0 then
  os.exit(0)
else
  os.exit(1)
end
