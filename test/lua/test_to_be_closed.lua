-- Copyright (c) 2021 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local test_suite = require "test_suite"

local major, minor = assert(_VERSION:match "^Lua (%d+)%.(%d+)$")
local version = major * 100 + minor

local suite = test_suite "test_to_be_closed"

function suite:test()
  if version < 504 then
    return test_skip()
  end

  assert(load [[
    local brigid = require "brigid"

    local writer = brigid.data_writer()

    do
      local writer_to_be_closed <close> = writer
      assert(writer:write "foo\n")
      assert(writer:write "bar\n")
      assert(writer:write "baz\n")
    end -- writer is closed

    -- attempt to use a closed
    local result, message = pcall(function () writer:write "qux\n" end)
    print(message)
    assert(not result)
  ]])()
end

return suite
