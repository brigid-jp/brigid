-- Copyright (c) 2019,2021,2024 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local brigid = require "brigid"
local test_suite = require "test_suite"

local registry = assert(debug.getregistry())
local is_love2d_data = registry["brigid.is_love2d_data"]
local string_to_ffi_pointer = registry["brigid.string_to_ffi_pointer"]

local function dump(source)
  if type(source) == "string" then
    local buffer = {}
    for i = 1, #source do
      buffer[i] = ("%02x"):format(source:byte(i))
    end
    return "string: " .. table.concat(buffer, " ")
  else
    return tostring(source)
  end
end

local suite = test_suite "test_common"
local debug = os.getenv "BRIGID_TEST_DEBUG" == "1"

function suite:test_is_love2d_data()
  local data_writer = brigid.data_writer()
  print(data_writer)

  local data, size = is_love2d_data(data_writer)
  assert(not data)
  assert(not size)
end

function suite:test_string_to_ffi_pointer()
  if not string_to_ffi_pointer then
    return test_skip()
  end

  local ffi = require "ffi"

  local s = "\1\2\3\4\5\6\7\8"
  if debug then print(dump(s)) end

  local p = assert(string_to_ffi_pointer(s))
  if debug then print(p) end
  assert(ffi.sizeof(p) == 8)

  local q = ffi.new("void*")
  if debug then print(q) end
  assert(ffi.sizeof(q) == 8)

  assert(ffi.typeof(p) == ffi.typeof(q))
  assert(p ~= q)
end

function suite:test_love()
  if not love then
    return test_skip()
  end

  local ffi = require "ffi"

  local byte_data = love.data.newByteData "foobarbazqux"
  local data, size = is_love2d_data(byte_data)
  if debug then print(dump(data), size) end
  assert(data)
  assert(size == 12)

  local p
  if type(data) == "string" then
    assert(string_to_ffi_pointer)
    p = string_to_ffi_pointer(data)
  else
    assert(type(data) == "userdata")
    p = data
  end

  ffi.copy(p, "ABCDEF", 6)
  if debug then print(byte_data:getString()) end
  assert(byte_data:getString() == "ABCDEFbazqux")
end

function suite:test_metatable1()
  local data_writer = brigid.data_writer()
  local metatable = getmetatable(data_writer)
  for k, v in pairs(metatable) do
    if debug then print(k, v) end
  end
  assert(metatable.__name == "brigid.data_writer")
end

function suite:test_metatable2()
  for k, v in pairs(registry) do
    if type(k) == "string" and k:find "^brigid%." then
      if debug then print(k, v) end
      if type(v) == "table" then
        assert(k == v.__name)
      end
    end
  end
end

function suite:test_ffi()
  local ffi
  pcall(function ()
    ffi = require "ffi"
  end)
  if not ffi then
    return test_skip()
  end

  local data_writer = brigid.data_writer()
  data_writer:write "foobarbazqux"
  assert(data_writer:get_string() == "foobarbazqux")

  ffi.copy(data_writer:get_pointer(), "ABCDEF", 6)
  if debug then print(data_writer:get_string()) end
  assert(data_writer:get_string() == "ABCDEFbazqux")
end

function suite:test_null1()
  if debug then print(brigid.null) end
  assert(type(brigid.null) == "userdata")
  assert(brigid.null ~= nil)
  assert(brigid.null ~= false)
end

function suite:test_null2()
  local cjson
  pcall(function ()
    cjson = require "cjson"
  end)
  if not cjson then
    return test_skip()
  end

  assert(brigid.null == cjson.null)
end

function suite:test_lightuserdata_bits()
  local bits = brigid.get_lightuserdata_bits()
  if debug then print(bits) end
  assert(bits == 32 or bits == 47 or bits == 64)
  if bits == 47 then
    assert(string_to_ffi_pointer)
  end
end

function suite:test_version()
  local version = brigid.get_version()
  if debug then print(version) end
  assert(version:find "^%d+%.%d+$")
end

function suite:test_dir1()
  local path = test_cwd.."/foo"
  assert(brigid.mkdir(path))
  local result, message = brigid.mkdir(path)
  if debug then print(message) end
  assert(not result)
  assert(brigid.rmdir(path))
end

function suite:test_dir2()
  local path = test_cwd
  local handle = assert(brigid.opendir(path))
  while true do
    local name = handle:read()
    if not name then
      break
    end
    if debug then print(name) end
  end
  assert(handle:close())
end

return suite
