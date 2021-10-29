-- Copyright (c) 2021 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local brigid = require "brigid"

local t = brigid.stopwatch()

t:start()
t:stop()
print(1, t:get_elapsed())

t:start()
t:stop() t:stop() t:stop() t:stop() t:stop() t:stop() t:stop() t:stop()
print(8, t:get_elapsed())

t:start()
t:stop() t:stop() t:stop() t:stop() t:stop() t:stop() t:stop() t:stop()
t:stop() t:stop() t:stop() t:stop() t:stop() t:stop() t:stop() t:stop()
print(16, t:get_elapsed())

t:start()
t:stop() t:stop() t:stop() t:stop() t:stop() t:stop() t:stop() t:stop()
t:stop() t:stop() t:stop() t:stop() t:stop() t:stop() t:stop() t:stop()
t:stop() t:stop() t:stop() t:stop() t:stop() t:stop() t:stop() t:stop()
t:stop() t:stop() t:stop() t:stop() t:stop() t:stop() t:stop() t:stop()
print(32, t:get_elapsed())
