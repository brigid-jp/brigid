-- Copyright (c) 2021,2024 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local brigid = require "brigid"
local test_suite = require "test_suite"

local suite = test_suite "test_json"
local debug = test_debug()

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

function suite:test_json_parse_boolean()
  assert(equal(brigid.json.parse "true", true))
  assert(equal(brigid.json.parse "false", false))
end

function suite:test_json_parse_null()
  assert(equal(brigid.json.parse "null", nil))
  assert(equal(brigid.json.parse("null", nil), nil))
  assert(equal(brigid.json.parse("null", false), false))
  assert(equal(brigid.json.parse("null", brigid.null), brigid.null))
end

function suite:test_json_parse_object()
  assert(equal(brigid.json.parse "{}", {}))
end

function suite:test_json_parse_array1()
  assert(equal(brigid.json.parse "[]", {}))
end

local source = [[
[ 11, 12, [ 21, 22, [ 31, 32, 33, 34 ], 23, 24 ], 33, 34 ]
]]

local expect = {
  11, 12, { 21, 22, { 31, 32, 33, 34 }, 23, 24 }, 33, 34
}

function suite:test_json_parse_array2()
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

function suite:test_json_parse_array3()
  assert(equal(brigid.json.parse(source), expect))
  assert(equal(brigid.json.parse(source, nil), expect))
end

function suite:test_json_parse_array4()
  local result = brigid.json.parse(source, false)
  assert(equal(result, { 1, false, 3, false, 5 }))
  assert(#result == 5)
end

function suite:test_json_parse_array5()
  local result = brigid.json.parse(source, brigid.null)
  assert(equal(result, { 1, brigid.null, 3, brigid.null, 5 }))
  assert(#result == 5)
end

function suite:test_json_parse_array6()
  local a = brigid.json.parse "[]"
  local metatable = getmetatable(a)
  assert(metatable.__index == table)
  assert(metatable.__name == "brigid.json.array")
  assert(metatable["brigid.json.array"])
  assert(#a == 0)
  a:insert(42)
  a:insert(69)
  assert(#a == 2)
  assert(a:concat "," == "42,69")
end

function suite:test_json_parse_array7()
  local a = brigid.json.parse "[42,69]"
  local metatable = getmetatable(a)
  assert(metatable.__index == table)
  assert(metatable.__name == "brigid.json.array")
  assert(metatable["brigid.json.array"])
  assert(#a== 2)
  assert(a:concat "," == "42,69")
end

function suite:test_json_parse_number1()
  assert(equal(brigid.json.parse "42", 42))
  assert(equal(brigid.json.parse "0.5", 0.5))
end

function suite:test_json_parse_number2()
  assert(equal(brigid.json.parse "0", 0))
  assert(equal(brigid.json.parse "0.0", 0))
  assert(equal(brigid.json.parse "0.0e0", 0))
  assert(equal(brigid.json.parse "0e0", 0))
  assert(equal(brigid.json.parse "-0", 0))
  assert(equal(brigid.json.parse "-0.0", 0))
  assert(equal(brigid.json.parse "-0.0e0", 0))
  assert(equal(brigid.json.parse "-0e0", 0))
end

function suite:test_json_parse_number3()
  assert(equal(brigid.json.parse "42", 42))
  assert(equal(brigid.json.parse "42.0", 42))
  assert(equal(brigid.json.parse "42.0e0", 42))
  assert(equal(brigid.json.parse "42e0", 42))
  assert(equal(brigid.json.parse "-42", -42))
  assert(equal(brigid.json.parse "-42.0", -42))
  assert(equal(brigid.json.parse "-42.0e0", -42))
  assert(equal(brigid.json.parse "-42e0", -42))
end

function suite:test_json_parse_number4()
  assert(equal(brigid.json.parse "6900", 6900))
  assert(equal(brigid.json.parse "6900.0", 6900))
  assert(equal(brigid.json.parse "69.0e2", 6900))
  assert(equal(brigid.json.parse "69e2", 6900))
  assert(equal(brigid.json.parse "-6900", -6900))
  assert(equal(brigid.json.parse "-6900.0", -6900))
  assert(equal(brigid.json.parse "-69.0e2", -6900))
  assert(equal(brigid.json.parse "-69e2", -6900))
end

function suite:test_json_parse_number5()
  assert(equal(brigid.json.parse "0.25", 0.25))
  assert(equal(brigid.json.parse "0.250", 0.25))
  assert(equal(brigid.json.parse "25e-2", 0.25))
  assert(equal(brigid.json.parse "2.5e-1", 0.25))
  assert(equal(brigid.json.parse "0.25e-0", 0.25))
  assert(equal(brigid.json.parse "-0.25", -0.25))
  assert(equal(brigid.json.parse "-0.250", -0.25))
  assert(equal(brigid.json.parse "-25e-2", -0.25))
  assert(equal(brigid.json.parse "-2.5e-1", -0.25))
  assert(equal(brigid.json.parse "-0.25e-0", -0.25))
end

function suite:test_json_parse_string1()
  assert(equal(brigid.json.parse [["foo"]], "foo"))
end

function suite:test_json_parse_string2()
  assert(equal(brigid.json.parse [["\u0001\u0000\u0002"]], "\1\0\2"))
end

function suite:test_json_parse_string3()
  assert(equal(brigid.json.parse [["あいうえお"]], "あいうえお"))
  assert(equal(brigid.json.parse [["あいうえお" ]], "あいうえお"))
  assert(equal(brigid.json.parse [[ "あいうえお"]], "あいうえお"))
  assert(equal(brigid.json.parse [[ "あいうえお" ]], "あいうえお"))
end

function suite:test_json_parse_string4()
  assert(equal(brigid.json.parse [["foo\"\\\/\b\f\n\r\tbar"]], "foo\"\\/\b\f\n\r\tbar"))
end

function suite:test_json_parse_string5()
  assert(equal(brigid.json.parse [["foo\n"]], "foo\n"))
  assert(equal(brigid.json.parse [["foo\nbar"]], "foo\nbar"))
  assert(equal(brigid.json.parse [["foo\nbar\n"]], "foo\nbar\n"))
  assert(equal(brigid.json.parse [["foo\nbar\nbaz"]], "foo\nbar\nbaz"))
  assert(equal(brigid.json.parse [["foo\nbar\nbaz\n"]], "foo\nbar\nbaz\n"))
end

function suite:test_json_parse_string6()
  assert(equal(brigid.json.parse [["\nfoo\n"]], "\nfoo\n"))
  assert(equal(brigid.json.parse [["\nfoo\nbar"]], "\nfoo\nbar"))
  assert(equal(brigid.json.parse [["\nfoo\nbar\n"]], "\nfoo\nbar\n"))
  assert(equal(brigid.json.parse [["\nfoo\nbar\nbaz"]], "\nfoo\nbar\nbaz"))
  assert(equal(brigid.json.parse [["\nfoo\nbar\nbaz\n"]], "\nfoo\nbar\nbaz\n"))
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

function suite:test_json_parse_rfc8259_3()
  local expect = string.char(0xF0, 0x9D, 0x84, 0x9E)
  assert(equal(brigid.json.parse [["\uD834\uDD1E"]], expect))
  assert(equal(brigid.json.parse [["\ud834\udd1e"]], expect))
end

function suite:test_json_parse_rfc3629_1()
  local expect = string.char(0x41, 0xE2, 0x89, 0xA2, 0xCE, 0x91, 0x2E)
  assert(equal(brigid.json.parse [["\u0041\u2262\u0391\u002E"]], expect))
  assert(equal(brigid.json.parse [["\u0041\u2262\u0391\u002e"]], expect))
end

function suite:test_json_parse_rfc3629_2()
  local expect = string.char(0xED, 0x95, 0x9C, 0xEA, 0xB5, 0xAD, 0xEC, 0x96, 0xB4)
  assert(equal(brigid.json.parse [["\uD55C\uAD6D\uC5B4"]], expect))
  assert(equal(brigid.json.parse [["\ud55c\uad6d\uc5b4"]], expect))
end

function suite:test_json_parse_rfc3629_3()
  local expect = string.char(0xE6, 0x97, 0xA5, 0xE6, 0x9C, 0xAC, 0xE8, 0xAA, 0x9E)
  assert(equal(brigid.json.parse [["\u65E5\u672C\u8A9E"]], expect))
  assert(equal(brigid.json.parse [["\u65e5\u672c\u8a9e"]], expect))
end

function suite:test_json_parse_locale1()
  os.setlocale "de_DE.UTF-8"
  local result, message = brigid.json.parse "[9223372036854775808,0e-19,1]"
  os.setlocale ""
  assert(result, message)
  assert(result[1] > 1)
  assert(result[2] == 0)
  assert(result[3] == 1)
end

function suite:test_json_parse_locale2()
  os.setlocale "de_DE.UTF-8"
  local result, message = brigid.json.parse "[0.5,1]"
  os.setlocale ""
  assert(result, message)
  assert(result[1] == 0.5)
  assert(result[2] == 1)
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

function suite:test_json_parse_integer4()
  if not math.type or math.maxinteger ~= 0x7FFFFFFFFFFFFFFF then
    return test_skip()
  end

  -- numeric_limits<int64_t>::digits10 == 18
  local v = brigid.json.parse "999999999999999999" -- 18 digits
  assert(math.type(v) == "integer")
  assert(v == 999999999999999999)

  local v = brigid.json.parse "-999999999999999999"
  assert(math.type(v) == "integer")
  assert(v == -999999999999999999)
end

function suite:test_json_parse_integer5()
  if not math.type or math.maxinteger ~= 0x7FFFFFFFFFFFFFFF then
    return test_skip()
  end

  -- numeric_limits<int64_t>::digits10 == 18
  local v = brigid.json.parse "9999999999999999999" -- 19 digits
  assert(math.type(v) == "float")

  local v = brigid.json.parse "-9999999999999999999"
  assert(math.type(v) == "float")
end

function suite:test_json_parse_integer6()
  if not math.type or math.maxinteger ~= 0x7FFFFFFFFFFFFFFF then
    return test_skip()
  end

  -- numeric_limits<int64_t>::digits10 == 18
  local v = brigid.json.parse "99999999999999999999" -- 19 digits
  assert(math.type(v) == "float")

  local v = brigid.json.parse "-99999999999999999999"
  assert(math.type(v) == "float")
end

function suite:test_json_parse_error1()
  local result, message = brigid.json.parse " { "
  if debug then print(message) end
  assert(not result)
end

function suite:test_json_parse_error2()
  local result, message = brigid.json.parse " { {} } "
  if debug then print(message) end
  assert(not result)
end

function suite:test_json_parse_error3()
  local result, message = brigid.json.parse " [ nan ] "
  if debug then print(message) end
  assert(not result)
end

function suite:test_json_parse_deep_array()
  local data_writer = brigid.data_writer()
  local depth = 1024
  for i = 1, depth do
    data_writer:write "["
  end
  data_writer:write(depth)
  for i = 1, depth do
    data_writer:write "]"
  end
  if debug then print(data_writer:get_string()) end
  local result = assert(brigid.json.parse(data_writer))
  local u = result
  for i = 1, depth do
    assert(type(u) == "table")
    assert(#u == 1)
    u = u[1]
  end
  assert(type(u) == "number")
  assert(u == depth)
end

function suite:test_json_parse_deep_object()
  local data_writer = brigid.data_writer()
  local depth = 1024
  for i = 1, depth do
    data_writer:write [[{"foo":]]
  end
  data_writer:write(depth)
  for i = 1, depth do
    data_writer:write "}"
  end
  if debug then print(data_writer:get_string()) end
  local result = assert(brigid.json.parse(data_writer))
  local u = result
  for i = 1, depth do
    assert(type(u) == "table")
    u = u.foo
  end
  assert(type(u) == "number")
  assert(u == depth)
end

function suite:test_json_write_and_parse1()
  local source = {
    Image = {
      Width = 800;
      Height = 600;
      Title = "View from 15th Floor";
      Thumbnail = {
        Url = "http://www.example.com/image/481989943";
        Height = 125;
        Width = 100;
      },
      Animated = false;
      IDs = { 116, 943, 234, 38793 };
    };
  }

  local result = brigid.data_writer():write_json(source):get_string()
  if debug then print(result) end
  assert(equal(brigid.json.parse(result), source))
end

return suite
