-- Copyright (c) 2019,2021 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local brigid = require "brigid"
local test_suite = require "test_suite"

local suite = test_suite "test_file_writer"
local debug = true

function suite:test_file_writer1()
  local writer = assert(brigid.file_writer(test_cwd .. "/test.dat"))
  assert(writer:write "foo\n")
  assert(writer:write "bar\n")
  assert(writer:write "baz\n")
  assert(writer:write "qux\n")
  assert(writer:close())
  assert(writer:close())

  local handle = assert(io.open(test_cwd .. "/test.dat"))
  assert(handle:read "*a" == "foo\nbar\nbaz\nqux\n")
  handle:close()
  os.remove "test.dat"
end

function suite:test_file_writer2()
  local result, message = brigid.file_writer(test_cwd .. "/no such directory/test.dat")
  if debug then print(message) end
  assert(not result)
end

return suite
