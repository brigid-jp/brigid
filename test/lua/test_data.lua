-- Copyright (c) 2021 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local brigid = require "brigid"
local test_suite = require "test_suite"

local suite = test_suite "test_data"
local debug = false

function suite:test_data1()
  local data_writer = brigid:data_writer()
  data_writer:write "foo"
  assert(data_writer:get_string() == "foo")
  data_writer:write(42)
  assert(data_writer:get_string() == "foo42")
  data_writer:write(0.5)
  assert(data_writer:get_string() == "foo420.5")
end

function suite:test_data2()
  local data_writer = brigid:data_writer()
  local result, message = pcall(function () data_writer:write {} end)
  if debug then print(message) end
  assert(not result)
  assert(message:find "bad self" or message:find "bad argument")
end

function suite:test_data3()
  local data_writer = brigid:data_writer()
  local result, message = pcall(function () data_writer:write(brigid.http_session {}) end)
  if debug then print(message) end
  assert(not result)
  assert(message:find "bad self" or message:find "bad argument")
end

function suite:test_data_love()
  if not love then
    return test_skip()
  end

  local data_writer = brigid:data_writer()
  data_writer:write(love.data.newByteData "foobarbazqux")
  assert(data_writer:get_string() == "foobarbazqux")
end

return suite
