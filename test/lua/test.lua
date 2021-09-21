-- Copyright (c) 2019,2021 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local test_suites = require "test_suites"

test_cwd = "."

local suites = test_suites()
if not suites() then
  error "test failed"
end
