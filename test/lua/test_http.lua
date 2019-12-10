-- Copyright (c) 2019 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local brigid = require "brigid"

local session = brigid.http_session {
  progress = function (now, total)
    print(now, total)
  end;

  -- header = function (code, header)
  --   print(code)
  --   for k, v in pairs(header) do
  --     print(k, v)
  --   end
  -- end;

  write = function (view)
    io.write(tostring(view))
  end;

  -- username = "";
  -- password = "";
}

local code, header = session:request {
  method = "POST";
  url = "https://brigid.jp/test/cgi/cat.cgi";
  header = {
    ["Content-Type"] = "application/octet-stream";
    ["User-Agent"] = "brigid/" .. brigid.get_version();
  };
  data = "foo=42";
}

io.write "\n"

print("status", code)
for k, v in pairs(header) do
  print(("%s: %s"):format(k, v))
end

