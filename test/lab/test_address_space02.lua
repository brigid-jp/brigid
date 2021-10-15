-- Copyright (c) 2021 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

package.cpath = ".libs/?.so;;"
local module = require "test_address_space"

print("check_full_range_lightuserdata", pcall(module.check_full_range_lightuserdata))

module.bench_get_registry01()
module.bench_get_registry02()
module.bench_get_registry03()
