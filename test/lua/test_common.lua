-- Copyright (c) 2019,2021 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local brigid = require "brigid"
local test_suite = require "test_suite"

local registry = assert(debug.getregistry())
local encode_pointer = registry["brigid.common.encode_pointer"]
local decode_pointer = registry["brigid.common.decode_pointer"]
local is_love2d_data = registry["brigid.common.is_love2d_data"]

local function hex(source)
  local buffer = {}
  for i = 1, #source do
    buffer[i] = ("%02x"):format(source:byte(i))
  end
  return table.concat(buffer, " ")
end

local suite = test_suite "test_common"

function suite:test()
  local session = brigid.http_session {}
  print(session)
  local encoded = encode_pointer(session)
  print(hex(encoded))
  local decoded = decode_pointer(encoded)
  print(decoded)

  local data, size = is_love2d_data(session)
  assert(not data)
  assert(not size)
end

function suite:test_love()
  if not love then
    return test_skip()
  end

  local byte_data = love.data.newByteData "foobarbazqux"
  local data, size = is_love2d_data(byte_data)
  print(hex(data))
  assert(data)
  assert(type(data) == "string")
  assert(size == 12)

  local ptr = decode_pointer(data)
  assert(ptr)
  assert(type(ptr) == "cdata")

  local ffi_ptr = byte_data:getFFIPointer()
  assert(ptr == ffi_ptr)

  local ffi_data = encode_pointer(ffi_ptr)
  assert(data == ffi_data)
end

function suite:test_metatable1()
  local data_writer = brigid.data_writer()
  local metatable = getmetatable(data_writer)
  for k, v in pairs(metatable) do
    print(k, v)
  end
  assert(metatable.__name == "brigid.data_writer")
end

function suite:test_metatable2()
  for k, v in pairs(registry) do
    if type(k) == "string" and k:find "^brigid%." and type(v) == "table" then
      print(k, v, v.__name)
      assert(k == v.__name)
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
  print(data_writer:get_string())
  assert(data_writer:get_string() == "ABCDEFbazqux")
end

return suite
