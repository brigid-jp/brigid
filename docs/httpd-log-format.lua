-- Copyright (c) 2019 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local source = {
  { remote_host = "%h" };
  { time = "%{%Y-%m-%d %H:%M:%S}t" };
  { method = "%m" };
  { path = "%U" };
  { query = "%q" };
  { protocol = "%H" };
  { authorization = "%{Authorization}i" };
  { cookie = "%{Cookie}i" };
  { host = "%{Host}i" };
  { referer = "%{Referer}i" };
  { ua = "%{User-Agent}i" };
  { status = "%s" };
  { content_type = "%{Content-Type}o" };
  { content_length = "%{Content-Length}o" };
  { size = "%B" };
  { size_in = "%I" };
  { size_out = "%O" };
  { usec = "%D" };
}

local result = {}

for i = 1, #source do
  local item = source[i]
  local k, v = next(item)
  result[i] = k .. ":" .. v
end

io.write("LogFormat \"", table.concat(result, "\\t"), "\" ltsv\n")
