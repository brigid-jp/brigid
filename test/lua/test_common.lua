-- Copyright (c) 2019,2021 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local brigid = require "brigid"
local test_suite = require "test_suite"

local suite = test_suite "test_common"

function suite:test()
  local registry = assert(debug.getregistry())
  local encode_pointer = registry["brigid.common.encode_pointer"]
  local decode_pointer = registry["brigid.common.decode_pointer"]
  local is_love2d_data = registry["brigid.common.is_love2d_data"]

  local session = brigid.http_session {}
  print(session)
  local encoded = encode_pointer(session)

  local buffer = {}
  for i = 1, #encoded do
    buffer[i] = ("%02x"):format(encoded:byte(i))
  end
  print(table.concat(buffer, " "))

  local decoded = decode_pointer(encoded)
  print(decoded)

  local data, size = is_love2d_data(session)
  assert(not data)
  assert(not size)
end

function suite:test_love()
  local registry = assert(debug.getregistry())
  local encode_pointer = registry["brigid.common.encode_pointer"]
  local decode_pointer = registry["brigid.common.decode_pointer"]
  local is_love2d_data = registry["brigid.common.is_love2d_data"]

  if not love then
    return
  end

  local byte_data = love.data.newByteData "foobarbazqux"
  local data, size = is_love2d_data(byte_data)
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

return suite
