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

function suite:test_json_parse_value1()
  assert(equal(brigid.json.parse "true", true))
end

function suite:test_json_parse_value2()
  assert(equal(brigid.json.parse "null", nil))
  assert(equal(brigid.json.parse("null", nil), nil))
  assert(equal(brigid.json.parse("null", false), false))
  assert(equal(brigid.json.parse("null", "null"), "null"))
end

function suite:test_json_parse_value3()
  assert(equal(brigid.json.parse "false", false))
end

function suite:test_json_parse_value4()
  assert(equal(brigid.json.parse "{}", {}))
end

function suite:test_json_parse_value5()
  assert(equal(brigid.json.parse "[]", {}))
end

function suite:test_json_parse_value6()
  assert(equal(brigid.json.parse "0.25", 0.25))
end

function suite:test_json_parse_value7()
  assert(equal(brigid.json.parse [["foo"]], "foo"))
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

function suite:test_json_parse_rfc8259_1()
  assert(equal(brigid.json.parse(source), expect))
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

function suite:test_json_parse_rfc8259_2()
  assert(equal(brigid.json.parse(source), expect))
end

local source = [[
[ 11, 12, [ 21, 22, [ 31, 32, 33, 34 ], 23, 24 ], 33, 34 ]
]]

local expect = {
  11, 12, { 21, 22, { 31, 32, 33, 34 }, 23, 24 }, 33, 34
}

function suite:test_json_parse_array1()
  assert(equal(brigid.json.parse(source), expect))
end

local source = [[
[ 1, null, 3, null, 5 ]
]]

local expect = {
  [1] = 1;
  [3] = 3;
  [5] = 5;
}

function suite:test_json_parse_array2()
  assert(equal(brigid.json.parse(source), expect))
  assert(equal(brigid.json.parse(source, nil), expect))
end

function suite:test_json_parse_array3()
  local result = brigid.json.parse(source, false)
  assert(equal(result, { 1, false, 3, false, 5 }))
  assert(#result == 5)
end

function suite:test_json_parse_array4()
  local result = brigid.json.parse(source, "")
  assert(equal(result, { 1, "", 3, "", 5 }))
  assert(#result == 5)
end

function suite:test_json_parse_string1()
  assert(equal(brigid.json.parse [["\u0001\u0000\u0002"]], "\1\0\2"))
end

function suite:test_json_parse_string2()
  assert(equal(brigid.json.parse [["あいうえお"]], "あいうえお"))
  assert(equal(brigid.json.parse [["あいうえお" ]], "あいうえお"))
  assert(equal(brigid.json.parse [[ "あいうえお"]], "あいうえお"))
  assert(equal(brigid.json.parse [[ "あいうえお" ]], "あいうえお"))
end

function suite:test_json_parse_string3()
  assert(equal(brigid.json.parse [["foo\"\\\/\b\f\n\r\tbar"]], "foo\"\\/\b\f\n\r\tbar"))
end

function suite:test_json_parse_string_rfc8259()
  local expect = string.char(0xF0, 0x9D, 0x84, 0x9E)
  assert(equal(brigid.json.parse [["\uD834\uDD1E"]], expect))
  assert(equal(brigid.json.parse [["\ud834\udd1E"]], expect))
end

function suite:test_json_parse_string_rfc3629_1()
  local expect = string.char(0x41, 0xE2, 0x89, 0xA2, 0xCE, 0x91, 0x2E)
  assert(equal(brigid.json.parse [["\u0041\u2262\u0391\u002E"]], expect))
  assert(equal(brigid.json.parse [["\u0041\u2262\u0391\u002e"]], expect))
end

function suite:test_json_parse_string_rfc3629_2()
  local expect = string.char(0xED, 0x95, 0x9C, 0xEA, 0xB5, 0xAD, 0xEC, 0x96, 0xB4)
  assert(equal(brigid.json.parse [["\uD55C\uAD6D\uC5B4"]], expect))
  assert(equal(brigid.json.parse [["\ud55c\uad6d\uc5b4"]], expect))
end

function suite:test_json_parse_string_rfc3629_3()
  local expect = string.char(0xE6, 0x97, 0xA5, 0xE6, 0x9C, 0xAC, 0xE8, 0xAA, 0x9E)
  assert(equal(brigid.json.parse [["\u65E5\u672C\u8A9E"]], expect))
  assert(equal(brigid.json.parse [["\u65e5\u672c\u8a9e"]], expect))
end

function suite:test_json_parse_number1()
  assert(equal(brigid.json.parse "0", 0))
  assert(equal(brigid.json.parse "0.0", 0))
  assert(equal(brigid.json.parse "0.0e0", 0))
  assert(equal(brigid.json.parse "0e0", 0))
  assert(equal(brigid.json.parse "-0", 0))
  assert(equal(brigid.json.parse "-0.0", 0))
  assert(equal(brigid.json.parse "-0.0e0", 0))
  assert(equal(brigid.json.parse "-0e0", 0))
end

function suite:test_json_parse_number2()
  assert(equal(brigid.json.parse "42", 42))
  assert(equal(brigid.json.parse "42.0", 42))
  assert(equal(brigid.json.parse "42.0e0", 42))
  assert(equal(brigid.json.parse "42e0", 42))
  assert(equal(brigid.json.parse "-42", -42))
  assert(equal(brigid.json.parse "-42.0", -42))
  assert(equal(brigid.json.parse "-42.0e0", -42))
  assert(equal(brigid.json.parse "-42e0", -42))
end

function suite:test_json_parse_number3()
  assert(equal(brigid.json.parse "6900", 6900))
  assert(equal(brigid.json.parse "6900.0", 6900))
  assert(equal(brigid.json.parse "69.0e2", 6900))
  assert(equal(brigid.json.parse "69e2", 6900))
  assert(equal(brigid.json.parse "-6900", -6900))
  assert(equal(brigid.json.parse "-6900.0", -6900))
  assert(equal(brigid.json.parse "-69.0e2", -6900))
  assert(equal(brigid.json.parse "-69e2", -6900))
end

function suite:test_json_parse_number4()
  local result = brigid.json.parse "[9223372036854775808,0e-19,1]"
  assert(result[1] > 1)
  assert(result[2] == 0)
end

function suite:test_json_parse_integer1()
  if not math.type then
    return test_skip()
  end

  local v = brigid.json.parse "42"
  assert(math.type(v) == "integer")
  assert(42)

  local s = ("%d"):format(math.maxinteger)
  local v = brigid.json.parse(s)
  assert(math.type(v) == "integer")
  assert(v == math.maxinteger)
end

function suite:test_json_parse_integer2()
  if not math.type or math.maxinteger ~= 0x7FFFFFFFFFFFFFFF then
    return test_skip()
  end

  local v = brigid.json.parse "9223372036854775807"
  assert(math.type(v) == "integer")
  assert(v == 0x7FFFFFFFFFFFFFFF)

  local v = brigid.json.parse "-9223372036854775808"
  assert(math.type(v) == "integer")
  assert(v == 0x8000000000000000)
end

function suite:test_json_parse_integer3()
  if not math.type or math.maxinteger ~= 0x7FFFFFFFFFFFFFFF then
    return test_skip()
  end

  local v = brigid.json.parse "9223372036854775808"
  assert(math.type(v) == "float")

  local v = brigid.json.parse "-9223372036854775809"
  assert(math.type(v) == "float")
end

function suite:test_json_parse_error1()
  local result, message = brigid.json.parse " { "
  print(message)
  assert(not result)
end

function suite:test_json_parse_error2()
  local result, message = brigid.json.parse " { {} } "
  print(message)
  assert(not result)
end

function suite:test_json_parse_error3()
  local result, message = brigid.json.parse " [ nan ] "
  print(message)
  assert(not result)
end

return suite
