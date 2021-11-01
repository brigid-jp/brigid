-- Copyright (c) 2021 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local brigid = require "brigid"
local test_suite = require "test_suite"

local suite = test_suite "test_stopwatch"
local debug = test_debug()

function suite:test_stopwatch1()
  local t = brigid.stopwatch()
  assert(t:start())
  assert(t:stop())
  assert(t:get_elapsed())
  assert(t:get_name())
  assert(t:get_resolution())
  if debug then print(t:get_elapsed(), t:get_name(), t:get_resolution()) end
end

function suite:test_stopwatch2()
  local t = brigid.stopwatch()
  local result, message = t:pcall(function () end)
  if debug then print(t:get_elapsed(), t:get_name(), t:get_resolution()) end
  assert(result)
  assert(not message)
end

function suite:test_stopwatch3()
  local t = brigid.stopwatch()
  local result, message = t:pcall(function () error "error" end)
  if debug then print(t:get_elapsed(), t:get_name(), t:get_resolution()) end
  assert(not result)
  assert(message)
end

function suite:test_stopwatch4()
  local t = brigid.stopwatch()
  local result, x, y, z = t:pcall(function (a, b, c) return c, a, b end, 17, 23, 37)
  if debug then print(t:get_elapsed(), t:get_name(), t:get_resolution()) end
  assert(result)
  assert(x == 37)
  assert(y == 17)
  assert(z == 23)
end

function suite:test_stopwatch5()
  local t = brigid.stopwatch()
  local result, x, y, z = t:pcall(function (a, b, c) return a + b + c end, 17, 23, 37)
  if debug then print(t:get_elapsed(), t:get_name(), t:get_resolution()) end
  assert(result)
  assert(x == 77)
  assert(not y)
  assert(not z)
end

function suite:test_stopwatch6()
  local names = brigid.get_stopwatch_names()
  assert(#names > 3)
  for i = 1, #names do
    local name = names[i]
    local t = brigid.stopwatch(name)
    assert(t:start())
    assert(t:stop())
    assert(t:get_elapsed())
    assert(t:get_name() == name)
    assert(t:get_resolution())
    if debug then print(t:get_elapsed(), t:get_name(), t:get_resolution()) end
  end
end

function suite:test_stopwatch7()
  local result, message = pcall(function () brigid.stopwatch "no such name" end)
  if debug then print(message) end
  assert(not result)
end

return suite
