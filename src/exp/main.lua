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

end

function love.draw()
  local width = love.window.getMode()
  love.graphics.printf(table.concat(text), 50, 50, width - 100)
end
