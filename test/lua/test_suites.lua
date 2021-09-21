-- Copyright (c) 2021 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local class = {}
local metatable = { __index = class }

local function new()
  return {
    (require "test_common");
    (require "test_version");
    (require "test_crypto");
    (require "test_writer");
    (require "test_to_be_closed");
  }
end

function metatable:__call(cwd)
  local pass = 0
  local fail = 0
  local skip = 0
  for i = 1, #self do
    local suite = self[i]
    suite:set_cwd(cwd)
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