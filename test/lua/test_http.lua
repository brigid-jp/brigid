-- Copyright (c) 2019 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local brigid = require "brigid"

local class = {}
local metatable = { __index = class }

function class.new(username, password)
  local self = {}
  self.session = assert(brigid.http_session {
    progress = function (now, total)
      return self:progress_cb(now, total)
    end;

    header = function (code, header)
      return self:header_cb(code, header)
    end;

    write = function (view)
      return self:write_cb(view)
    end;

    username = username;
    password = password;
  })
  return self
end

function class:progress_cb(now, total)
  print(("%s %s %5.1f%%"):format(self.method, self.url, now * 100 / total))
end

function class:header_cb(code, header)
  self.code = code
  self.header = header
end

function class:write_cb(view)
  local buffer = self.buffer
  buffer[#buffer + 1] = view:get_string()
end

function class:request(method, url, header, request)
  self.buffer = {}
  self.method = method
  self.url = url

  if not request then
    request = {}
  end
  request.method = method
  request.url = url
  request.header = header
  local result, message = self.session:request(request)
  if not result then
    return result, message
  end

  local code = self.code
  local header = self.header
  local body = table.concat(self.buffer)

  self.buffer = nil
  self.code = nil
  self.header = nil

  return code, header, body
end

function class:request_data(method, url, header, data)
  return self:request(method, url, header, { data = data })
end

function class:request_file(method, url, header, file)
  return self:request(method, url, header, { file = file })
end

function class:close()
  return self.session:close()
end

local test_client = setmetatable(class, {
  __call = function (_, username, password)
    return setmetatable(class.new(username, password), metatable)
  end;
})

local data4k = ("0123456789ABCDE\n"):rep(4096 / 16)
local out = assert(io.open("test.dat", "wb"))
for i = 1, 1024 * 1024 / #data4k do
  out:write(data4k)
end
out:close()

local client = test_client()

local code, header, body = client:request("GET", "https://brigid.jp/")
assert(code == 200)
assert(header["Content-Length"] == "0")
assert(header["Content-Type"] == "text/html; charset=UTF-8")
assert(body == "")

local data = "foo\nbar\nbaz\nqux\n";

local code, header, body = client:request_data("PUT", "https://brigid.jp/test/dav/auth-none/test.txt", nil, data)
assert(code == 201 or code == 204)
if code == 204 then
  assert(body == "")
end

local code, header, body = client:request("GET", "https://brigid.jp/test/dav/auth-none/test.txt");
assert(code == 200)
assert(body == data)

local code, header, body = client:request("HEAD", "https://brigid.jp/test/dav/auth-none/test.txt");
assert(code == 200)
assert(body == "")

local code, header, body = client:request("DELETE", "https://brigid.jp/test/dav/auth-none/test.txt");
assert(code == 204)
assert(body == "")

local code, header, body = client:request("GET", "https://brigid.jp/test/dav/auth-none/test.txt")
assert(code == 404)

local code, header, body = client:request("HEAD", "https://brigid.jp/test/dav/auth-none/test.txt")
assert(code == 404)
assert(body == "")

local code, header, body = client:request_file("PUT", "https://brigid.jp/test/dav/auth-none/test.txt", nil, "test.dat")
assert(code == 201 or code == 204)

local code, header, body = client:request_file("GET", "https://brigid.jp/test/dav/auth-none/test.txt")
assert(code == 200)
assert(body == data4k:rep(1024 * 1024 / #data4k))

local code, header, body = client:request_file("DELETE", "https://brigid.jp/test/dav/auth-none/test.txt")
assert(code == 204)
assert(body == "")

local code, header, body = client:request("GET", "https://brigid.jp/test/cgi/redirect.cgi?count=1")
assert(code == 200)
assert(body == "ok\n")
assert(not header.Locatuion)

local code, header, body = client:request("GET", "https://brigid.jp/test/cgi/redirect.cgi?count=11")
assert(code == 200)
assert(body == "ok\n")
assert(not header.Locatuion)

local result, message = client:request("GET", "https://brigid.jp/test/cgi/redirect.cgi?count=21")
print(message)
assert(not result)

local code, header, body = client:request("GET", "https://brigid.jp/test/dav/auth-basic/")
assert(code == 401)

client:close()
local result, message = pcall(function () client:request("GET", "https://brigid.jp/") end)
print(message)
assert(not result)

local client = test_client("brigid", "O6jIOchrWCGuOSB4")
local code, header, body = client:request("GET", "https://brigid.jp/test/dav/auth-basic/")
assert(code == 200)
client:close()

local client = test_client()

local data = [[{"foo":42}]]
local request_header = { ["Content-Type"] = "application/json; charset=UTF-8" }

local code, header, body = client:request_data("POST", "https://brigid.jp/test/cgi/cat.cgi", request_header, data)
assert(code == 200)
assert(body == data)

local code, header, body = client:request_data("POST", "https://brigid.jp/test/cgi/env.cgi", request_header, data)
assert(code == 200)
assert(body:find("CONTENT_TYPE=application/json; charset=UTF-8\n", 1, true))

local result, message = client:request("GET", "https://133.242.153.239/")
print(message)
assert(not result)

local code, header, body = client:request("GET", "https://brigid.jp/test/cgi/nph-header.cgi")
assert(code == 200)
assert(header["X-Test1"] == "foo bar")
assert(header["X-Test2"] == "foo bar")
assert(header["X-Test3"] == "foo  bar")
assert(header["X-Test4"]:find "^foo[ \t]+bar$")
assert(header["X-Test5"]:find "^foo[ \t]+bar$")
assert(header["X-Test6"] == "foo bar")
assert(header["x-test7"] == "foo bar")
assert(header["X-tEsT8"] == "foo bar")

local data = ""
local request_header = { ["Content-Type"] = "application/x-www-form-urlencoded" }

local code, header, body = client:request_data("POST", "https://brigid.jp/test/cgi/cat.cgi", request_header, data)
assert(code == 200)
assert(body == "")

local code, header, body = client:request_data("POST", "https://brigid.jp/test/cgi/env.cgi", request_header, data)
assert(code == 200)
assert(body:find("CONTENT_TYPE=application/x-www-form-urlencoded\n", 1, true))

local result, message = client:request("GET", "https://no-such-host.brigid.jp/")
print(message)
assert(not result)

local result, message = client:request("GET", "!!! invalid url !!!")
print(message)
assert(not result)

local code, header, body = client:request("POST", "https://brigid.jp/test/%63%67%69/env.cgi?%20%21")
assert(code == 200)
assert(body:find("QUERY_STRING=%20%21\n", 1, true))

local code, header, body = client:request("GET", "https://brigid.jp/test/cgi/cookie.cgi")
assert(code == 200)
assert(header["Set-Cookie"] == "code=42; Path=/test/cgi/; Secure")
assert(body == "HTTP_COOKIE=\n")

local code, header, body = client:request("GET", "https://brigid.jp/test/cgi/cookie.cgi")
assert(code == 200)
assert(header["Set-Cookie"] == "code=42; Path=/test/cgi/; Secure")
assert(body == "HTTP_COOKIE=\n")

local code, header, body = client:request("GET", "https://brigid.jp/test/cgi/cookie.cgi", { Cookie = "code=69" })
assert(code == 200)
assert(not header["Set-Cookie"])
assert(body == "HTTP_COOKIE=code=69\n")

local ua = "brigid/" .. brigid.get_version()
local code, header, body = client:request("GET", "https://brigid.jp/test/cgi/env.cgi", { ["User-Agent"] = ua })
assert(code == 200)
assert(body:find("USER_AGENT=" .. ua .. "\n", 1, true))

client.session:close()
client.session:close()

--
-- explicitly cancel in the progress callback
--
-- NSURLSession backend is not canceled immediately

local canceling = 0

local session = brigid.http_session {
  progress = function (now, total)
    print(("progress %5.1f%%"):format(now * 100 / total))
    if now * 2 >= total then
      print "explicitly cancel in the progress callback"
      canceling = canceling + 1
      return false
    end
  end;

  header = function ()
    canceling = canceling + 1
  end;

  write = function ()
    canceling = canceling + 1
  end;
}
local result, message = session:request {
  method = "PUT",
  url = "https://brigid.jp/test/dav/auth-none/test.txt";
  file = "test.dat";
}
print(message, canceling)
assert(not result)
assert(message == "canceled")
assert(canceling == 1)

-- explicitly cancel in the header callback

-- explicitly cancel in the write callback

os.remove "test.dat"
