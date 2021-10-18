R""--(
-- Copyright (c) 2019,2021 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local class, lightuserdata_mask_is_not_zero = ...

if lightuserdata_mask_is_not_zero then
  local ffi = require "ffi"

  function class.string_to_ffi_pointer(source)
    local buffer = ffi.new "void*[2]"
    ffi.copy(buffer, source)
    return buffer[0]
  end

  function class.is_love2d_data(source)
    if source.getPointer and source.getSize then
      if source.getFFIPointer then
        local result = source:getFFIPointer()
        if result ~= nil then
          -- ffi pointer to string
          local buffer = ffi.new("void*[1]", result)
          return ffi.string(buffer, ffi.sizeof(buffer))
        end
      end
      return source:getPointer(), source:getSize()
    end
  end
else
  function class.is_love2d_data(source)
    if source.getPointer and source.getSize then
      return source:getPointer(), source:getSize()
    end
  end
end

--)"--"
