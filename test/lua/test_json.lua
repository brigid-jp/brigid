-- Copyright (c) 2021 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local brigid = require "brigid"
local test_suite = require "test_suite"

local suite = test_suite "test_json"

function suite:test_json_decode1()
  local r = brigid.json.decode "true"
  print(r)
end

function suite:test_json_decode2()
  local r = brigid.json.decode "[ true , false ]"
  print(r)
  for k, v in pairs(r) do
    print(k, v)
  end
end

function suite:test_json_decode3()
  local r = brigid.json.decode "\"foo\""
  print(r)
end

function suite:test_json_decode4()
  local r = brigid.json.decode [[{ "foo": true, "bar": "あいうえお\tかきくけこ" }]]
  print(r)
  for k, v in pairs(r) do
    print(k, v)
  end
end

function suite:test_json_decode5()
  local r = brigid.json.decode " 3.14"
  print(r, type(r), math.type(r))
end

local cjson_example3 = [[
{"widget": {
    "debug": "on",
    "window": {
        "title": "Sample Konfabulator Widget",
        "name": "main_window",
        "width": 500,
        "height": 500
    },
    "image": {
        "src": "Images/Sun.png",
        "name": "sun1",
        "hOffset": 250,
        "vOffset": 250,
        "alignment": "center"
    },
    "text": {
        "data": "Click Here",
        "size": 36,
        "style": "bold",
        "name": "text1",
        "hOffset": 250,
        "vOffset": 100,
        "alignment": "center",
        "onMouseUp": "sun1.opacity = (sun1.opacity / 100) * 90;"
    }
}}
]]

function suite:test_json_decode_cjson_example3()
  local r = brigid.json.decode(cjson_example3)
  print(r, type(r), math.type(r))
end

return suite
