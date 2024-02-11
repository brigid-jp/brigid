-- Copyright (c) 2019,2021,2024 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local brigid = require "brigid"
local test_suites = require "test_suites"

brigid.dlopen_self()

test_cwd = "."

local suites = test_suites()
if not suites() then
  error "test failed"
end
