R""--(
-- Copyright (c) 2019 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php
local impl_get_pointer_ffi

local ffi
pcall(function () ffi = require "ffi" end)
if not ffi then
  return
end

ffi.cdef [[
typedef const void* (*get_pointer_ffi_t)(void*);
]]

local get_pointer_ffi = ffi.cast("get_pointer_ffi**", impl_pointer_ffi)[0]

print("get_pointer_ffi", get_pointer_ffi)

return function (view)
  return get_pointer_ffi(view)
end
--)"--"
