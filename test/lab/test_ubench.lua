#! /usr/bin/env lua

-- Copyright (c) 2021 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

package.cpath = "../../src/lua/.libs/?.so;;"
local brigid = require "brigid"

local stopwatch = brigid.ubench.stopwatch()
stopwatch:start()
for i = 1, 1000000 do
  stopwatch:stop()
end
