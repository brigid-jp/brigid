-- Copyright (c) 2019 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

-- generate
-- export hpp
-- export sh
-- export bat

local command = ...
local zone_size = 32

local function read()
  local source = io.read(zone_size)
  assert(#source == zone_size)
  local result = {}
  for i = 1, zone_size, 4 do
    local a, b, c, d = source:byte(i, i + 3)
    local v = a * 0x1000000 + b * 0x10000 + c * 0x100 + d
    result[#result + 1] = a * 0x1000000 + b * 0x10000 + c * 0x100 + d
  end
  return result
end

local commands = {}

function commands.help()
  io.write [[
lua brigid_zone.lua generate >brigid_zone.bin
lua brigid_zone.lua export_hpp <brigid_zone.bin >brigid_zone.hpp
lua brigid_zone.lua export_sh  <brigid_zone.bin >brigid_zone.sh
lua brigid_zone.lua export_dos <brigid_zone.bin >brigid_zone.bat
]]
end

-- dd if=/dev/urandom of=brigiz_zone.bin bs=32 count=1
-- openssl rand 32 -out brigid_zone.bin
function commands.generate()
  local handle = assert(io.open("/dev/urandom", "rb"))
  io.write(handle:read(zone_size))
end

function commands.import_env()
  local zone = {
    tonumber(os.getenv "BRIGID_ZONE1") or 0;
    tonumber(os.getenv "BRIGID_ZONE2") or 0;
    tonumber(os.getenv "BRIGID_ZONE3") or 0;
    tonumber(os.getenv "BRIGID_ZONE4") or 0;
    tonumber(os.getenv "BRIGID_ZONE5") or 0;
    tonumber(os.getenv "BRIGID_ZONE6") or 0;
    tonumber(os.getenv "BRIGID_ZONE7") or 0;
    tonumber(os.getenv "BRIGID_ZONE8") or 0;
  }
  for i = 1, #zone do
    local a = zone[i]
    local d = a % 0x100
    a = (a - d) / 0x100
    local c = a % 0x100
    a = (a - c) / 0x100
    local b = a % 0x100
    a = (a - b) / 0x100
    io.write(string.char(a, b, c, d))
  end
end

function commands.export_hpp()
  local zone = read()
  for i = 1, #zone do
    io.write(([[
#if !(BRIGID_ZONE%d+0)
#undef BRIGID_ZONE%d
#define BRIGID_ZONE%d 0x%08X
#endif
]]):format(i, i, i, zone[i]))
  end
end

function commands.export_sh()
  local zone = read()
  for i = 1, #zone do
    io.write(([[
BRIGID_ZONE%d=0x%08X
]]):format(i, zone[i]))
  end
  io.write "export"
  for i = 1, #zone do
    io.write((" BRIGID_ZONE%d"):format(i))
  end
  io.write "\n"
end

function commands.export_dos()
  local zone = read()
  for i = 1, #zone do
    io.write(([[
set BRIGID_ZONE%d=0x%08X
]]):format(i, zone[i]))
  end
end

local f = commands[command]
if f then
  f(...)
else
  commands.help()
end
