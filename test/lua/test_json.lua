-- Copyright (c) 2021 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local brigid = require "brigid"
local test_suite = require "test_suite"

local suite = test_suite "test_json"

function suite:test_json_parser1()
  local parser = brigid.json_parser()
  local r = parser:parse("true")
  print(r)
end

function suite:test_json_parser2()
  local parser = brigid.json_parser()
  local r = parser:parse("[ true , false ]")
  print(r)
  for k, v in pairs(r) do
    print(k, v)
  end
end

function suite:test_json_parser3()
  local parser = brigid.json_parser()
  local r = parser:parse("\"foo\"")
  print(r)
end

function suite:test_json_parser4()
  local parser = brigid.json_parser()
  local r = parser:parse [[{ "foo": true, "bar": "あいうえお\tかきくけこ" }]]
  print(r)
  for k, v in pairs(r) do
    print(k, v)
  end
end

function suite:test_json_parser5()
  local parser = brigid.json_parser()
  local r = parser:parse(" 314e0 ")
  print(r, type(r), math.type(r))
end

return suite
