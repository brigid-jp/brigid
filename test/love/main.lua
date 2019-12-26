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

    local ptr = ffi.new("const void*[2]")
    write("test3 ", ffi.sizeof(ptr), " ", tostring(ptr), "\n")
    write("test4 ", ffi.sizeof(ptr[0]), " ", tostring(ptr[0]), "\n")
    ffi.copy(ptr, str);
    write("test5 ", ffi.sizeof(ptr), " ", tostring(ptr), "\n")
    write("test6 ", ffi.sizeof(ptr[0]), " ", tostring(ptr[0]), "\n")
  end

  write("package.cpath ", package.cpath, "\n")
  package.cpath = "../lua/.libs/?.so;" .. package.cpath
  write("package.cpath ", package.cpath, "\n")
  local brigid
  pcall(function () brigid = require "brigid" end)
  write("brigid ", tostring(brigid), "\n")
  if brigid then
    write("brigid.version ", brigid.get_version(), "\n")

    local plaintext = love.data.newByteData "The quick brown fox jumps over the lazy dog"
    local key = love.data.newByteData "01234567890123456789012345678901"
    local iv = love.data.newByteData "0123456789012345"
    local ciphertext = love.data.newByteData(table.concat {
      "\224\111\099\167\017\232\183\170";
      "\159\148\064\016\125\070\128\161";
      "\023\153\067\128\234\049\210\162";
      "\153\185\083\002\212\057\185\112";
      "\044\142\101\169\146\054\236\146";
      "\007\004\145\092\241\169\138\068";
    })
    local out = love.data.newByteData(plaintext:getSize())
    love.filesystem.write("test1.dat", out)
    local cryptor = brigid.decryptor("aes-256-cbc", key, iv, function (view)
      write("view {", view:get_string(), "}\n")
      ffi.copy(out:getFFIPointer(), view:get_pointer(), view:get_size())
    end)
    cryptor:update(ciphertext, true)
    love.filesystem.write("test2.dat", out)
  end

  if brigid and brigid.hasher then
    local hasher = brigid.hasher "sha256"
    hasher:update "The quick brown fox jumps over the lazy dog"
    local result = hasher:digest()
    for i = 1, #result do
      write(("%02X"):format(result:byte(i)))
    end
    write "\n"
  end
end

function love.draw()
  local width = love.window.getMode()
  love.graphics.printf(table.concat(text), 50, 50, width - 100)
end
