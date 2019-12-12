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

  if data.getFFIPointer then
    write("getPointer is_void_pointer ", tostring(ffi.istype("void*", data:getPointer())), "\n")
    write("getPointer is_const_void_pointer ", tostring(ffi.istype("const void*", data:getPointer())), "\n")
    write("getFFIPointer is_void_pointer ", tostring(ffi.istype("void*", data:getFFIPointer())), "\n")
    write("getFFIPointer is_const_void_pointer ", tostring(ffi.istype("const void*", data:getFFIPointer())), "\n")
    -- local ptr = ffi.new("const void*[1]", data:getPointer())
    local ptr = ffi.new("const void*[1]", data:getFFIPointer())
    write("test1 ", ffi.sizeof(ptr), " ", tostring(ptr), "\n")
    write("test2 ", ffi.sizeof(ptr[0]), " ", tostring(ptr[0]), "\n")
    local str = ffi.string(ptr, ffi.sizeof(ptr))
    for i = 1, #str do
      if i > 1 then
        write " "
      end
      write(("%02x"):format(str:byte(i, i)))
    end
    write "\n"

    local ptr = ffi.new("const void*[1]")
    write("test3 ", ffi.sizeof(ptr), " ", tostring(ptr), "\n")
    write("test4 ", ffi.sizeof(ptr[0]), " ", tostring(ptr[0]), "\n")
    ffi.copy(ptr, str);
    write("test5 ", ffi.sizeof(ptr), " ", tostring(ptr), "\n")
    write("test6 ", ffi.sizeof(ptr[0]), " ", tostring(ptr[0]), "\n")
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
