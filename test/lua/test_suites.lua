-- Copyright (c) 2021 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local class = {}
local metatable = { __index = class }

local test_suite_names = {
  "test_common";
  "test_version";
  "test_crypto";
  "test_writer";
  "test_to_be_closed";
  "test_http";
  "test_json";
  "test_data";
  "test_view";
}

local function new()
  local pattern = os.getenv "BRIGID_TEST_SUITE_NAME_PATTERN"
  local self = {}
  for i = 1, #test_suite_names do
    local name = test_suite_names[i]
    if not pattern or name:find(pattern) then
      self[#self + 1] = require(name)
    end
  end
  return self
end

function metatable:__call()
  local pass = 0
  local fail = 0
  local skip = 0
  for i = 1, #self do
    local suite = self[i]
    pass, fail, skip = suite(pass, fail, skip)
  end
  local total = pass + fail + skip

  print "============================================================"
  print("TOTAL: " .. total)
  print("PASS: " .. pass)
  print("FAIL: " .. fail)
  print("SKIP: " .. skip)
  print "============================================================"

  return fail == 0
end

return setmetatable(class, {
  __call = function ()
    return setmetatable(new(), metatable)
  end;
})
