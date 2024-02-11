#! /usr/bin/env lua

-- Copyright (c) 2024 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local brigid = require "brigid"

local function write(out)
  -- ...
end

local module_path = brigid.dladdr()
print(module_path)
local result, message = brigid.dlopen(module_path)
-- local result, message = brigid.dlopen "brigid.so"
-- print(result, message)

local session = brigid.http_session { write = write }
session:close()
os.exit(true, true)
