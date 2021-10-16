-- Copyright (c) 2021 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

package.cpath = ".libs/?.so;;"
local module = require "test_address_space"

local function dump(v)
  local t = type(v)
  if t == "userdata" then
    print(v)
  elseif t == "string" then
    local b = { v:byte(1, #v) }
    for i = 1, #b do
      b[i] = ("%02X"):format(b[i])
    end
    print("string: " .. table.concat(b, " "))
  else
    print(type(v), v)
  end
  return v
end

local ffi
pcall(function () ffi = require "ffi" end)
if ffi then
  -- pointer to string
  local function encode_pointer(source)
    local buffer = ffi.new("void*[1]", source)
    return ffi.string(buffer, ffi.sizeof(buffer))
  end

  -- string to pointer (cdata)
  local function decode_pointer(source)
    local buffer = ffi.new "void*[2]"
    ffi.copy(buffer, source)
    return buffer[0]
  end

  local h_lud = dump(module.get_handle_lightuserdata())
  local h_str = dump(module.get_handle_string())
  local h_cdt = dump(decode_pointer(h_str))

  dump(encode_pointer(h_lud))
  dump(encode_pointer(h_cdt))

  ffi.copy(h_lud, "foo")
  print(ffi.string(h_lud, 3))

  io.write "--\n"
  module.dump_handle(h_lud);
  module.dump_handle(h_cdt);
  module.dump_handle(h_str);
end
