#! /usr/bin/env lua

-- Copyright (c) 2024 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local brigid = require "brigid"

local function write(out)
  -- ...
end

print(brigid.dlopen_self())

local session = brigid.http_session { write = write }
session:close()
os.exit(true, true)
