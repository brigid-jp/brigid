#! /usr/bin/env lua

-- Copyright (c) 2019 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local count
local query_string = os.getenv "QUERY_STRING"
if query_string then
  count = tonumber(query_string:match "^count=(%d+)$") or 0
end

if count <= 1 then
  io.write "Content-Type: text/plain; charset=UTF-8\n\nok\n"
else
  io.write(("Location: https://brigid.jp/test/cgi/redirect.cgi?count=%d\r\n\r\n"):format(count - 1))
end
