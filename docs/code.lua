-- Copyright (c) 2019 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local mode, name = ...

local data = io.read "*a"
local size = #data

if mode == "cxx" then
  io.write(("static const char %s_data[] = {"):format(name))

  for i = 1, size do
    local r = i % 8
    if r == 1 then
      io.write "\n  "
    else
      io.write " "
    end
    io.write(([['\x%02X',]]):format(data:byte(i)))
  end
  io.write "\n};\n"

  io.write(("static const std::string %s(%s_data, %d);\n"):format(name, name, size))
elseif mode == "lua" then
  io.write(("local %s = table.concat {"):format(name))
  for i = 1, size do
    local r = i % 8
    if r == 1 then
      if i == 1 then
        io.write "\n  \""
      else
        io.write "\";\n  \""
      end
    end
    io.write(([[\%03d]]):format(data:byte(i)))
  end
  io.write "\";\n}\n"
end
