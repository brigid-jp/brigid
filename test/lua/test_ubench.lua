-- Copyright (c) 2021 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local brigid = require "brigid"
local test_suite = require "test_suite"

local suite = test_suite "test_ubench"
local debug = true

function suite:test_ubench()
  print(brigid.ubench.check_runtime())
end

function suite:test_ubench_stopwatch1()
  local t = brigid.ubench.stopwatch()
  t:start()
  t:stop()
  if debug then print(t:get_elapsed()) end
end

function suite:test_ubench_stopwatch2()
  local t = brigid.ubench.stopwatch()
  t:start()
  t:stop() t:stop() t:stop() t:stop() t:stop() t:stop() t:stop() t:stop()
  if debug then print(t:get_elapsed()) end
end

function suite:test_ubench_stopwatch3()
  local t = brigid.ubench.stopwatch()
  t:start()
  t:stop() t:stop() t:stop() t:stop() t:stop() t:stop() t:stop() t:stop()
  t:stop() t:stop() t:stop() t:stop() t:stop() t:stop() t:stop() t:stop()
  if debug then print(t:get_elapsed()) end
end

local f_start = brigid.ubench.stopwatch.start
local f_stop = brigid.ubench.stopwatch.stop
local f_get_elapsed = brigid.ubench.stopwatch.get_elapsed

function suite:test_ubench_stopwatch4()
  local t = brigid.ubench.stopwatch()
  f_start(t)
  f_stop(t)
  if debug then print(f_get_elapsed(t)) end
end

function suite:test_ubench_stopwatch5()
  local t = brigid.ubench.stopwatch()
  f_start(t)
  f_stop(t) f_stop(t) f_stop(t) f_stop(t) f_stop(t) f_stop(t) f_stop(t) f_stop(t)
  if debug then print(f_get_elapsed(t)) end
end

function suite:test_ubench_stopwatch6()
  local t = brigid.ubench.stopwatch()
  f_start(t)
  f_stop(t) f_stop(t) f_stop(t) f_stop(t) f_stop(t) f_stop(t) f_stop(t) f_stop(t)
  f_stop(t) f_stop(t) f_stop(t) f_stop(t) f_stop(t) f_stop(t) f_stop(t) f_stop(t)
  if debug then print(f_get_elapsed(t)) end
end

return suite
