-- Copyright (c) 2021 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local brigid = require "brigid"
local test_suite = require "test_suite"

local suite = test_suite "test_view"
local debug = false

function suite:test_view()
  local ffi
  pcall(function () ffi = require "ffi" end)

  local closed_view
  local cryptor = assert(brigid.encryptor("aes-256-cbc", ("a"):rep(32), ("b"):rep(16), function (view)
    local p = assert(view:get_pointer())
    local n = assert(view:get_size())
    local s = assert(view:get_string())
    local t = assert(tostring(view))

    if ffi then
      assert(type(p) == "cdata")
    else
      assert(type(p) == "userdata")
    end

    assert(n == #s)
    assert(s == t)

    if ffi then
      assert(ffi.string(p, n) == s)
    end

    closed_view = view
  end))

  for i = 1, 16 do
    assert(cryptor:update(("c"):rep(257)))
  end
  assert(cryptor:update(("d"):rep(257), true))

  local result, message = pcall(function () closed_view:get_pointer() end)
  if debug then print(message) end
  assert(not result)

  local result, message = pcall(function () closed_view:get_size() end)
  if debug then print(message) end
  assert(not result)

  local result, message = pcall(function () closed_view:get_string() end)
  if debug then print(message) end
  assert(not result)

  local result, message = pcall(function () tostring(closed_view) end)
  if debug then print(message) end
  assert(not result)
end

return suite
