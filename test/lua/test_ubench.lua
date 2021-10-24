-- Copyright (c) 2021 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local brigid = require "brigid"
local test_suite = require "test_suite"

local suite = test_suite "test_ubench"
local debug = true

function suite:test_ubench_stopwatch1()
  local t = brigid.ubench.stopwatch()
  t:start()
  while true do
    t:stop()
    if t:get_elapsed() > 0 then
      break
    end
  end
  if debug then print(t:get_elapsed()) end
end

return suite
