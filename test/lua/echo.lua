-- Copyright (c) 2019 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

function handle(r)
  local args = r:parseargs()

  local keys = {}
  if args.keys then
    for key in (args.keys .. ","):gmatch "(.-)," do
      keys[#keys + 1] = key
    end
  end

  r.content_type = "text/plain"
  r:puts(("%s\n"):format(r.the_request))
  for i = 1, #keys do
    local k = keys[i]
    local v = r.headers_in[k] or ""
    r:puts(("%s: %s\n"):format(k, v))
  end
  r:puts "\n"
  if r.method == "POST" or r.method == "PUT" then
    r:puts((r:requestbody()))
  end
  return apache2.OK
end
