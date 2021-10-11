-- Copyright (c) 2021 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local brigid = require "brigid"
local test_suite = require "test_suite"

local suite = test_suite "test_json"

local function equal(self, that)
  if self == that then
    return true
  end
  if type(self) == "table" and type(that) == "table" then
    for k, v in pairs(self) do
      if not equal(v, that[k]) then
        return false
      end
    end
    for k, v in pairs(that) do
      if that[k] == nil then
        return false
      end
    end
    return true
  end
end

function suite:test_json_decode_value1()
  assert(equal(brigid.json.decode "true", true))
end

function suite:test_json_decode_value2()
  assert(equal(brigid.json.decode "null", nil))
end

function suite:test_json_decode_value3()
  assert(equal(brigid.json.decode "false", false))
end

function suite:test_json_decode_value4()
  assert(equal(brigid.json.decode "{}", {}))
end

function suite:test_json_decode_value5()
  assert(equal(brigid.json.decode "[]", {}))
end

function suite:test_json_decode_value6()
  assert(equal(brigid.json.decode "0.25", 0.25))
end

function suite:test_json_decode_value7()
  assert(equal(brigid.json.decode [["foo"]], "foo"))
end

local source = [[
{
  "Image": {
      "Width":  800,
      "Height": 600,
      "Title":  "View from 15th Floor",
      "Thumbnail": {
          "Url":    "http://www.example.com/image/481989943",
          "Height": 125,
          "Width":  100
      },
      "Animated" : false,
      "IDs": [116, 943, 234, 38793]
    }
}
]]

local expect = {
  Image = {
    Width = 800;
    Height = 600;
    Title = "View from 15th Floor";
    Thumbnail = {
      Url = "http://www.example.com/image/481989943";
      Height = 125;
      Width = 100;
    };
    Animated = false;
    IDs = { 116, 943, 234, 38793 };
  };
}

function suite:test_json_decode_rfc8259_1()
  assert(equal(brigid.json.decode(source), expect))
end

local source = [[
[
  {
     "precision": "zip",
     "Latitude":  37.7668,
     "Longitude": -122.3959,
     "Address":   "",
     "City":      "SAN FRANCISCO",
     "State":     "CA",
     "Zip":       "94107",
     "Country":   "US"
  },
  {
     "precision": "zip",
     "Latitude":  37.371991,
     "Longitude": -122.026020,
     "Address":   "",
     "City":      "SUNNYVALE",
     "State":     "CA",
     "Zip":       "94085",
     "Country":   "US"
  }
]
]]

local expect = {
  {
    precision = "zip";
    Latitude = 37.7668;
    Longitude = -122.3959;
    Address = "";
    City = "SAN FRANCISCO";
    State = "CA";
    Zip = "94107";
    Country = "US";
  };
  {
    precision = "zip";
    Latitude = 37.371991;
    Longitude = -122.026020;
    Address = "";
    City = "SUNNYVALE";
    State = "CA";
    Zip = "94085";
    Country = "US";
  };
}

function suite:test_json_decode_rfc8259_2()
  assert(equal(brigid.json.decode(source), expect))
end

function suite:test_json_decode_string()
  assert(equal(brigid.json.decode [["\u0001\u0000\u0002"]], "\1\0\2"))
end

function suite:test_json_decode_string_rfc8259()
  local expect = string.char(0xF0, 0x9D, 0x84, 0x9E)
  assert(equal(brigid.json.decode [["\uD834\uDD1E"]], expect))
  assert(equal(brigid.json.decode [["\ud834\udd1E"]], expect))
end

function suite:test_json_decode_string_rfc3629_1()
  local expect = string.char(0x41, 0xE2, 0x89, 0xA2, 0xCE, 0x91, 0x2E)
  assert(equal(brigid.json.decode [["\u0041\u2262\u0391\u002E"]], expect))
  assert(equal(brigid.json.decode [["\u0041\u2262\u0391\u002e"]], expect))
end

function suite:test_json_decode_string_rfc3629_2()
  local expect = string.char(0xED, 0x95, 0x9C, 0xEA, 0xB5, 0xAD, 0xEC, 0x96, 0xB4)
  assert(equal(brigid.json.decode [["\uD55C\uAD6D\uC5B4"]], expect))
  assert(equal(brigid.json.decode [["\ud55c\uad6d\uc5b4"]], expect))
end

function suite:test_json_decode_string_rfc3629_3()
  local expect = string.char(0xE6, 0x97, 0xA5, 0xE6, 0x9C, 0xAC, 0xE8, 0xAA, 0x9E)
  assert(equal(brigid.json.decode [["\u65E5\u672C\u8A9E"]], expect))
  assert(equal(brigid.json.decode [["\u65e5\u672c\u8a9e"]], expect))
end

return suite
