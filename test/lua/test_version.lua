-- Copyright (c) 2019 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local brigid = require "brigid"

assert(brigid.get_version():match "^%d+%.%d+$")
print(brigid.get_version())
