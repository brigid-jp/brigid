#! /usr/bin/env lua

-- Copyright (c) 2019 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local cookie = os.getenv "HTTP_COOKIE" or ""

if cookie == "" then
  io.write "Set-Cookie: code=42; Path=/test/cgi/; Secure\n"
end

io.write(([[
Content-Type: text/plain; charset=UTF-8

HTTP_COOKIE=%s
]]):format(cookie))
