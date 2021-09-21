-- Copyright (c) 2021 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local test_suites = require "test_suites"

local suites = test_suites()
if suites "." then
  os.exit(0)
else
  os.exit(1)
end
