R""--(
-- Copyright (c) 2019,2021 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local class, no_full_range_lightuserdata = ...

if no_full_range_lightuserdata then
  local ffi = require "ffi"

  local function encode_pointer(source)
    local buffer = ffi.new("void*[1]", source)
    return ffi.string(buffer, ffi.sizeof(buffer))
  end

  function class.decode_pointer(source)
    local buffer = ffi.new "void*[2]"
    ffi.copy(buffer, source)
    return buffer[0]
  end

  function class.is_love2d_data(source)
    if source.getPointer and source.getSize then
      if source.getFFIPointer then
        local result = source:getFFIPointer()
        if result ~= nil then
          return encode_pointer(result), source:getSize()
        end
      end
      return encode_pointer(source:getPointer()), source:getSize()
    end
  end
else
  function class.is_love2d_data(source)
    if source.getPointer and source.getSize then
      return encode_pointer(source:getPointer()), source:getSize()
    end
  end
end

-- local ffi
-- pcall(function () ffi = require "ffi" end)
-- 
-- if ffi then
--   function class.encode_pointer(source)
--     local buffer = ffi.new("void*[1]", source)
--     return ffi.string(buffer, ffi.sizeof(buffer))
--   end
-- 
--   function class.decode_pointer(source)
--     local buffer = ffi.new "void*[2]"
--     ffi.copy(buffer, source)
--     return buffer[0]
--   end
-- end
-- 
-- function class.is_love2d_data(source)
--   if source.getPointer and source.getSize then
--     if source.getFFIPointer then
--       local result = source:getFFIPointer()
--       if result ~= nil then
--         return class.encode_pointer(result), source:getSize()
--       end
--     end
--     return class.encode_pointer(source:getPointer()), source:getSize()
--   end
-- end

--
--)"--"
