R""--(
-- Copyright (c) 2019 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local class = ...

local ffi
pcall(function () ffi = require "ffi" end)
if not ffi then
  return nil
end

function class.encode_pointer(source)
  local buffer = ffi.new("void*[1]", source)
  return ffi.string(buffer, ffi.sizeof(buffer))
end

function class.decode_pointer(source)
  local buffer = ffi.new "void*[1]"
  ffi.copy(buffer, source)
  return buffer[0]
end

--
--)"--"
