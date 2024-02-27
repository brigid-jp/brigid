-- Copyright (c) 2021,2024 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local brigid = require "brigid"
local test_suite = require "test_suite"

local suite = test_suite "test_data_writer"
local debug = os.getenv "BRIGID_TEST_DEBUG" == "1"

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

function suite:test_write_urlencoded1()
  local data_writer = brigid.data_writer():write_urlencoded "日本語"
  local result = assert(data_writer:get_string())
  if debug then print(result) end
  assert(result == "%E6%97%A5%E6%9C%AC%E8%AA%9E")
end

function suite:test_write_urlencoded2()
  local expect = "%00%01%02%03%04%05%06%07%08%09%0A%0B%0C%0D%0E%0F%10%11%12%13%14%15%16%17%18%19%1A%1B%1C%1D%1E%1F+%21%22%23%24%25%26%27%28%29*%2B%2C-.%2F0123456789%3A%3B%3C%3D%3E%3F%40ABCDEFGHIJKLMNOPQRSTUVWXYZ%5B%5C%5D%5E_%60abcdefghijklmnopqrstuvwxyz%7B%7C%7D%7E%7F"

  local source = {}
  for i = 0, 127 do
    source[i + 1] = i
  end
  local source = string.char((table.unpack or unpack)(source))

  local data_writer = brigid.data_writer():write_urlencoded(source)
  local result = assert(data_writer:get_string())
  if debug then print(result) end
  assert(result == expect)
end

function suite:test_write_urlencoded3()
  local expect = "%E3%82%AD%E3%83%BC1=%E5%80%A41&%E3%82%AD%E3%83%BC2=%E5%80%A42&%E3%82%AD%E3%83%BC3=%E5%80%A43"

  local data_writer = brigid.data_writer()
  data_writer:write_urlencoded("キー1"):write"=":write_urlencoded("値1")
    :write"&":write_urlencoded("キー2"):write"=":write_urlencoded("値2")
    :write"&":write_urlencoded("キー3"):write"=":write_urlencoded("値3")
  local result = assert(data_writer:get_string())
  if debug then print(result) end
  assert(result == expect)
end

function suite:test_write_json_string1()
  local expect = [["\u0000\u0001\u0002\u0003\u0004\u0005\u0006\u0007\b\t\n\u000B\f\r\u000E\u000F\u0010\u0011\u0012\u0013\u0014\u0015\u0016\u0017\u0018\u0019\u001A\u001B\u001C\u001D\u001E\u001F !\"#$%&'()*+,-.\/0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\u007F"]]

  local source = {}
  for i = 0, 127 do
    source[i + 1] = i
  end
  local source = string.char((table.unpack or unpack)(source))

  local data_writer = assert(brigid.data_writer():write_json_string(source))
  local result = assert(data_writer:get_string())
  if debug then print(result) end
  assert(result == expect)
  assert(brigid.json.parse(result) == source)
end

function suite:test_write_json_string2()
  local source1 = string.char(
    0x41, 0xE2, 0x89, 0xA2, 0xCE, 0x91, 0x2E, -- U+0041 U+2262 U+0391 U+002E
    0xED, 0x95, 0x9C, 0xEA, 0xB5, 0xAD, 0xEC, 0x96, 0xB4, -- U+D55C U+AD6D U+C5B4
    0xE6, 0x97, 0xA5, 0xE6, 0x9C, 0xAC, 0xE8, 0xAA, 0x9E, -- U+65E5 U+672C U+8A9E
    0xEF, 0xBB, 0xBF, 0xF0, 0xA3, 0x8E, 0xB4, -- U+233B4
    0xE2, 0x80, 0xA7) -- U+2027
  local source2 = string.char(
    0xE2, 0x80, 0xA8, -- U+2028
    0xE2, 0x80, 0xA9) -- U+2029
  local source3 = string.char(
    0xE2, 0x80, 0xAA) -- U+202A

  local source = source1..source2..source2..source3
  local expect = "\""..source1.."\\u2028\\u2029\\u2028\\u2029"..source3.."\""

  local data_writer = assert(brigid.data_writer():write_json_string(source))
  local result = assert(data_writer:get_string())
  if debug then print(result) end
  assert(result == expect)
  assert(brigid.json.parse(result) == source)
end

function suite:test_write_json_string3()
  local data_writer = assert(brigid.data_writer():write_json_string "")
  local result = assert(data_writer:get_string())
  if debug then print(result) end
  assert(result == [[""]])
end

function suite:test_write_json_number1()
  local data_writer = brigid.data_writer()

  assert(data_writer:write_json_number(17)):write ","
  assert(data_writer:write_json_number(42.0)):write ","
  assert(data_writer:write_json_number(69.125)):write ","

  local result, message = pcall(function ()
    data_writer:write_json_number "foobarbaz"
  end)
  if debug then print(message) end
  assert(not result)

  local inf = 1 / 0
  local result, message = pcall(function ()
    data_writer:write_json_number(inf)
  end)
  if debug then print(message) end
  assert(not result)

  local nan = 0 / 0
  local result, message = pcall(function ()
    data_writer:write_json_number(nan)
  end)
  if debug then print(message) end
  assert(not result)

  local pzero = 1 / inf
  local mzero = -1 / inf
  assert(data_writer:write_json_number(pzero)):write ","
  assert(data_writer:write_json_number(mzero)):write ","

  local result = assert(data_writer:get_string())
  if debug then print(result) end
  assert(result == "17,42,69.125,0,0,")
end

function suite:test_write_json_number2()
  if not math.type or math.maxinteger ~= 0x7FFFFFFFFFFFFFFF then
    return test_skip()
  end

  local data_writer = brigid.data_writer()

  -- numeric_limits<int64_t>::digits10 == 18
  local v = 999999999999999999 -- 18 digits
  assert(math.type(v) == "integer")
  assert(data_writer:write_json_number(v)):write ","
  assert(data_writer:write_json_number(-v)):write ","
  local result = data_writer:get_string()
  if debug then print(result) end
  assert(result == "999999999999999999,-999999999999999999,")
end

function suite:test_write_json_number3()
  if not math.type or math.maxinteger ~= 0x7FFFFFFFFFFFFFFF then
    return test_skip()
  end

  local data_writer = brigid.data_writer()

  -- numeric_limits<int64_t>::digits10 == 18
  local v = 9999999999999999999 -- 19 digits
  assert(math.type(v) == "float")
  assert(data_writer:write_json_number(v)):write ","
  assert(data_writer:write_json_number(-v)):write ","
  local result = data_writer:get_string()
  if debug then print(result) end
  assert(result == "1e+19,-1e+19,")
end

function suite:test_write_json()
  local result = brigid.data_writer():write_json {
    a = {
      brigid.json.array();
      {};
      { "日本語\n" };
      { true, false, brigid.null };
      { 17, 42.0, 69.125 };
      { "", "あいうえおかきくけこさしすせそ" };
    };
  }:get_string()

  if debug then print(result) end
  assert(result == [=[{"a":[[],{},["日本語\n"],[true,false,null],[17,42,69.125],["","あいうえおかきくけこさしすせそ"]]}]=])
end

return suite
