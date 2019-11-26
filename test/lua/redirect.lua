-- Copyright (c) 2019 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

function handle(r)
  local args = r:parseargs()
  local count = tonumber(args.count) or 0

  if count <= 1 then
    r.content_type = "text/plain"
    r:puts "ok\n"
    return apache2.OK
  else
    r.headers_out["Location"] = r:construct_url(("%s?count=%d"):format(r.uri, count - 1))
    return apache2.HTTP_MOVED_TEMPORARILY
  end
end
