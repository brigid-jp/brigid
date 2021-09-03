-- Copyright (c) 2019 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local brigid = require "brigid"

local url = ...

io.write "return {\n"

for i = 2, #arg do
  local filename = arg[i]
  local handle = assert(io.open(filename, "rb"))
  local data = handle:read "*a"
  handle:close()

  local hasher = assert(brigid.hasher "sha256")
  hasher:update(data)
  local digest = hasher:digest()
  local buffer = {}
  for i = 1, #digest do
    buffer[i] = ("\\%03d"):format(digest:byte(i))
  end

  io.write(([[
  {
    url = "%s%s";
    size = %d;
    sha256 = "%s";
  };
]]):format(url, filename, #data, table.concat(buffer)))
end

io.write "}\n"
