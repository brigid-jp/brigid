-- Copyright (c) 2019 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local brigid = require "brigid"

local writer = assert(brigid.data_writer())
assert(writer:write "foo\n")
assert(writer:write "bar\n")
assert(writer:write "baz\n")
assert(writer:write "qux\n")
assert(writer:get_string() == "foo\nbar\nbaz\nqux\n")
assert(writer:get_size() == 16)
assert(writer:close())
local result, message = pcall(function () writer:get_size() end)
print(message)
assert(not result)

local writer = assert(brigid.file_writer "test.dat")
assert(writer:write "foo\n")
assert(writer:write "bar\n")
assert(writer:write "baz\n")
assert(writer:write "qux\n")
assert(writer:close())

local handle = assert(io.open "test.dat")
assert(handle:read "*a" == "foo\nbar\nbaz\nqux\n")
handle:close()
os.remove "test.dat"

local writer, message = brigid.file_writer "no such directory/test.dat"
print(message)
assert(not writer)
