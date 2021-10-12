-- Copyright (c) 2021 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local brigid = require "brigid"
local test_suite = require "test_suite"

local suite = test_suite "test_json"

function suite:test_data1()
  local data_writer = brigid:data_writer()
  data_writer:write "foo"
end

function suite:test_data2()
  local data_writer = brigid:data_writer()
  local result, message = pcall(function ()
    data_writer:write {}
  end)
  print(message)
  assert(not result)
end

function suite:test_data3()
  local data_writer = brigid:data_writer()
  local result, message = pcall(function ()
    data_writer:write(brigid.http_session {})
  end)
  print(message)
  assert(not result)
end

function suite:test_data4()
  local data_writer = brigid:data_writer()
  data_writer:write "foo"
  data_writer:write(data_writer)
  data_writer:write(data_writer)
  data_writer:write(data_writer)
  assert(data_writer:get_string() == ("foo"):rep(8))
end

return suite
