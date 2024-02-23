-- Copyright (c) 2021,2024 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local brigid = require "brigid"
local test_suite = require "test_suite"

local suite = test_suite "test_data_writer"
local debug = true

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

return suite
