-- Copyright (c) 2021 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

package.cpath = ".libs/?.so;;"
local module = require "test_address_space"

module.create_native_threads(10, [[
local thread_id = ...
-- print(thread_id)

package.cpath = ".libs/?.so;;"

-- io.write "module load begin\n"
local module = require "test_address_space"
-- io.write "module load end\n"

]])
