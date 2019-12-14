-- Copyright (c) 2019 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local brigid = require "brigid"

local writer = brigid.data_writer()
writer:write "foo\n"
writer:write "bar\n"
writer:write "baz\n"
writer:write "qux\n"
assert(writer:get_string() == "foo\nbar\nbaz\nqux\n")
assert(writer:get_size() == 16)

writer:close()
local result, message = pcall(function() writer:get_size() end)
print(message)
assert(not result)
