-- Copyright (c) 2019 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local ffi = require "ffi"
local love = love

local text = {}

local function write(...)
  local data = {...}
  for i = 1, #data do
    text[#text + 1] = data[i]
  end
end

function love.load()
  local data = love.data.newByteData("foo bar baz qux")
  write("pointer ", tostring(data:getPointer()), "\n")
  if data.getFFIPointer then
    write("ffipointer ", tostring(data:getFFIPointer()), "\n")
  end
  write("size ", data:getSize(), "\n")
  write("string [[", data:getString(), "]]\n")

  local data2 = love.data.newByteData "FUCKINGOM "

  ffi.copy(data:getFFIPointer(), data2:getFFIPointer(), 4);

  write("string [[", data:getString(), "]]\n")

  local metatable = getmetatable(data)
  write("metatable ", tostring(metatable), "\n")
  for k, v in pairs(metatable) do
    write("  ", tostring(k), "=", tostring(v), "\n")
  end

  local registry = debug.getregistry()
  write("registry ", tostring(registry), "\n")
  -- for k, v in pairs(registry) do
  --   write("  ", tostring(k), "=", tostring(v), "\n")
  -- end

  if rawequal(registry.ByteData, metatable) then
    write "rawequal(registry.ByteData, metatable)\n"
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

end

function love.draw()
  local width = love.window.getMode()
  love.graphics.printf(table.concat(text), 50, 50, width - 100)
end
