-- Copyright (c) 2021 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local brigid = require "brigid"
local test_suite = require "test_suite"

local suite = test_suite "test_stopwatch"
local debug = true

function suite:test_stopwatch1()
  local t = brigid.stopwatch()
  t:start()
  t:stop()
  if debug then print(t:get_elapsed()) end
end

function suite:test_stopwatch2()
  local t = brigid.stopwatch()
  t:start()
  t:stop() t:stop() t:stop() t:stop() t:stop() t:stop() t:stop() t:stop()
  if debug then print(t:get_elapsed()) end
end

function suite:test_stopwatch3()
  local t = brigid.stopwatch()
  t:start()
  t:stop() t:stop() t:stop() t:stop() t:stop() t:stop() t:stop() t:stop()
  t:stop() t:stop() t:stop() t:stop() t:stop() t:stop() t:stop() t:stop()
  if debug then print(t:get_elapsed()) end
end

local f_start = brigid.stopwatch.start
local f_stop = brigid.stopwatch.stop
local f_get_elapsed = brigid.stopwatch.get_elapsed

function suite:test_stopwatch4()
  local t = brigid.stopwatch()
  f_start(t)
  f_stop(t)
  if debug then print(f_get_elapsed(t)) end
end

function suite:test_stopwatch5()
  local t = brigid.stopwatch()
  f_start(t)
  f_stop(t) f_stop(t) f_stop(t) f_stop(t) f_stop(t) f_stop(t) f_stop(t) f_stop(t)
  if debug then print(f_get_elapsed(t)) end
end

function suite:test_stopwatch6()
  local t = brigid.stopwatch()
  f_start(t)
  f_stop(t) f_stop(t) f_stop(t) f_stop(t) f_stop(t) f_stop(t) f_stop(t) f_stop(t)
  f_stop(t) f_stop(t) f_stop(t) f_stop(t) f_stop(t) f_stop(t) f_stop(t) f_stop(t)
  if debug then print(f_get_elapsed(t)) end
end

function suite:test_stopwatch_get_name1()
  local t = brigid.stopwatch()
  if debug then print(t:get_name()) end
  if debug then print(t:get_resolution()) end
end

function suite:test_stopwatch_get_name2()
  local t = brigid.stopwatch "std::chrono::steady_clock"
  if debug then print(t:get_name()) end
  t:start()
  t:stop()
  if debug then print(t:get_elapsed()) end
end

function suite:test_stopwatch_get_name3()
  local result, message = pcall(function () brigid.stopwatch "no_such_name" end)
  assert(not result)
  if debug then print(message) end
end

function suite:test_get_stopwatch_names()
  local names = brigid.get_stopwatch_names()
  for i = 1, #names do
    if debug then print(names[i]) end
  end
end

return suite
