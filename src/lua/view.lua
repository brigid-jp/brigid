R""--(
-- Copyright (c) 2019 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local ffi
pcall(function () ffi = require "ffi" end)
if not ffi then
  return nil
end

ffi.cdef [[
typedef struct {
  const void* (*f)(void*);
} brigid_view_get_pointer_ffi_t;
]]

local get_pointer_ffi = ffi.cast("brigid_view_get_pointer_ffi_t**", (...))[0].f

return function (self)
  return get_pointer_ffi(self)
end
--)"--"
