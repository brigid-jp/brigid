-- Copyright (c) 2019 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local brigid = require "brigid"

local registry = assert(debug.getregistry())
local encode_pointer = registry["brigid.common.encode_pointer"]
local decode_pointer = registry["brigid.common.decode_pointer"]

local session = brigid.http_session {}
print(session)
local encoded = encode_pointer(session)

for i = 1, #encoded do
  if i > 1 then
    io.write " "
  end
  io.write(("%02x"):format(encoded:byte(i)))
end
io.write "\n"

local decoded = decode_pointer(encoded)
print(decoded)
