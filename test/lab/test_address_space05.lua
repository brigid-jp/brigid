-- Copyright (c) 2021 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

package.cpath = ".libs/?.so;;"
local module = require "test_address_space"

local a = 0
local b = 0
local c = 0
local d = 0

module.bench_callback01(
  function (v) a = a + v end,
  function (v) b = b + v end,
  function (v) c = c + v end,
  function (v) d = d + v end)

local a = 0
local b = 0
local c = 0
local d = 0

module.bench_callback02(
  function (v) a = a + v end,
  function (v) b = b + v end,
  function (v) c = c + v end,
  function (v) d = d + v end)

