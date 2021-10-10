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
  print(r, type(r))
end

function suite:test_json_string1()
  local r = brigid.json.decode [["\u0001\u0000\u0002"]]
  assert(r == "\1\0\2")
end

function suite:test_json_string2()
  local r = brigid.json.decode [["\uD834\uDD1E"]]
  assert(r == string.char(0xF0, 0x9D, 0x84, 0x9E))
end

function suite:test_json_string3()
  local r = brigid.json.decode [["\u0041\u2262\u0391\u002E"]]
  assert(r == string.char(0x41, 0xE2, 0x89, 0xA2, 0xCE, 0x91, 0x2E))
end

function suite:test_json_string4()
  local r = brigid.json.decode [["\uD55C\uAD6D\uC5B4"]]
  assert(r == string.char(0xED, 0x95, 0x9C, 0xEA, 0xB5, 0xAD, 0xEC, 0x96, 0xB4))
end

function suite:test_json_string5()
  local r = brigid.json.decode [["\u65E5\u672C\u8A9E"]]
  assert(r == string.char(0xE6, 0x97, 0xA5, 0xE6, 0x9C, 0xAC, 0xE8, 0xAA, 0x9E))
end

local cjson_example2 = [[
{"menu": {
  "id": "file",
  "value": "File",
  "popup": {
    "menuitem": [
      {"value": "New", "onclick": "CreateNewDoc()"},
      {"value": "Open", "onclick": "OpenDoc()"},
      {"value": "Close", "onclick": "CloseDoc()"}
    ]
  }
}}
]]

function suite:test_json_decode_cjson_example2()
  local r = brigid.json.decode(cjson_example2)
  print(r, type(r))
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
  print(r, type(r))
end

return suite
