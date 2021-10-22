-- Copyright (c) 2021 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local class = {}
local metatable = { __index = class }

local private_impl = function () end
local private_skip = function () end

assert(test_cwd)

function test_skip()
  error(private_skip)
end

local function new(name)
  return {
    [private_impl] = {
      name = name;
      case_keys = {};
      cases = {};
    }
  }
end

function metatable:__newindex(key, value)
  local impl = self[private_impl]
  local case_keys = impl.case_keys
  local cases = impl.cases

  assert(not cases[key])
  case_keys[#case_keys + 1] = key
  cases[key] = value
end

function metatable:__call(pass, fail, skip)
  local impl = self[private_impl]
  local case_keys = impl.case_keys
  local cases = impl.cases

  local suite_name = impl.name
  for i = 1, #case_keys do
    local key = case_keys[i]
    local result, message = pcall(cases[key], self)
    if result then
      pass = pass + 1
      print("[PASS] " .. suite_name .. "." .. key)
    elseif message == private_skip then
      skip = skip + 1
      print("[SKIP] " .. suite_name .. "." .. key)
    else
      fail = fail + 1
      print("[FAIL] " .. suite_name .. "." .. key .. ": " .. message)
    end
  end
  return pass, fail, skip
end

return setmetatable(class, {
  __call = function (_, name)
    return setmetatable(new(name), metatable)
  end;
})
