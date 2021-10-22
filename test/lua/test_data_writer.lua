-- Copyright (c) 2021 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local brigid = require "brigid"
local test_suite = require "test_suite"

local suite = test_suite "test_data_writer"
local debug = false

function suite:test_data_writer1()
  local ffi
  pcall(function () ffi = require "ffi" end)

  local data_writer = assert(brigid.data_writer())
  assert(data_writer:write "foobarbazqux")

  local p = assert(data_writer:get_pointer())
  local n = assert(data_writer:get_size())
  local m = assert(#data_writer)
  local s = assert(data_writer:get_string())
  local t = assert(tostring(data_writer))

  assert(n == m)
  assert(n == #s)
  assert(s == t)
  assert(s == "foobarbazqux")

  if ffi then
    assert(ffi.string(p, n) == s)
  end

  assert(data_writer:close())
  assert(data_writer:close())

  local result, message = pcall(function () data_writer:get_pointer() end)
  if debug then print(message) end
  assert(not result)
  assert(message:find "bad self" or message:find "bad argument")

  local result, message = pcall(function () data_writer:get_size() end)
  if debug then print(message) end
  assert(not result)
  assert(message:find "bad self" or message:find "bad argument")

  local result, message = pcall(function () data_writer:get_string() end)
  if debug then print(message) end
  assert(not result)
  assert(message:find "bad self" or message:find "bad argument")

  local result, message = pcall(function () tostring(data_writer) end)
  if debug then print(message) end
  assert(not result)
  assert(message:find "bad self" or message:find "bad argument")

  local result, message = pcall(function () data_writer:write "foobarbazqux" end)
  if debug then print(message) end
  assert(not result)
  assert(message:find "bad self" or message:find "bad argument")
end

function suite:test_data_writer2()
  local data_writer = brigid.data_writer()
  data_writer:write "foobar"
  for i = 1, 8 do
    data_writer:write(data_writer)
  end

  local n = assert(data_writer:get_size())
  local m = assert(#data_writer)
  local s = assert(data_writer:get_string())
  local t = assert(tostring(data_writer))

  assert(n == m)
  assert(n == #s)
  assert(s == t)
  assert(s == ("foobar"):rep(256))
end

function suite:test_data_writer3()
  local data_writer = brigid.data_writer():reserve(1024):write "foobarbaz"
  assert(data_writer:get_string() == "foobarbaz")

  local result, message = pcall(function () data_writer:reserve(-1) end)
  if debug then print(message) end
  assert(not result)
  assert(message:find "bad self" or message:find "bad argument")
end

return suite
