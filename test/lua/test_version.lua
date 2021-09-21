-- Copyright (c) 2019,2021 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local brigid = require "brigid"
local test_suite = require "test_suite"

local suite = test_suite "test_version"

function suite:test()
  local version = brigid.get_version()
  print(brigid.get_version())
  assert(version:match "^%d+%.%d+$")
end

return suite
