-- Copyright (c) 2021 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local brigid = require "brigid"

local url, version = ...

local function process(system, arch, suffix)
  local filename = ("brigid-%s-%s-%s.%s"):format(version, system, arch, suffix)
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
    %s = {
      url = "%s%s";
      filename = "brigid.%s";
      size = %d;
      sha256 = "%s";
    };
]]):format(arch, url, filename, suffix, #data, table.concat(buffer)))
end

io.write [[return {
  ["OS X"] = {
]]

process("osx", "x64", "so")

io.write [[
  };
  Windows = {
]]

process("win", "x64", "dll")
process("win", "x86", "dll")

io.write [[
  };
}
]]
