-- Copyright (c) 2019,2021 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local brigid = require "brigid"
local test_http_client = require "test_http_client"
local test_suite = require "test_suite"

local data4k = ("0123456789ABCDE\n"):rep(4096 / 16)

local suite = test_suite "test_http"

local client

function suite:test_create_data()
  local out = assert(io.open(test_cwd .. "/test.dat", "wb"))
  for i = 1, 1024 * 1024 / #data4k do
    out:write(data4k)
  end
  out:close()
end

function suite:test_open_client_noauth()
  client = test_http_client()
end

function suite:test1()
  local code, header, body = client:request("GET", "https://brigid.jp/")
  assert(code == 200)
  assert(header["Content-Length"] == "0")
  assert(header["Content-Type"] == "text/html; charset=UTF-8")
  assert(body == "")
end

local data = "foo\nbar\nbaz\nqux\n";

function suite:test2()
  local code, header, body = client:request_data("PUT", "https://brigid.jp/test/dav/auth-none/test.txt", nil, data)
  assert(code == 201 or code == 204)
  if code == 204 then
    assert(body == "")
  end
end

function suite:test3()
  local code, header, body = client:request("GET", "https://brigid.jp/test/dav/auth-none/test.txt");
  assert(code == 200)
  assert(body == data)
end

function suite:test4()
  local code, header, body = client:request("HEAD", "https://brigid.jp/test/dav/auth-none/test.txt");
  assert(code == 200)
  assert(body == "")
end

function suite:test5()
  local code, header, body = client:request("DELETE", "https://brigid.jp/test/dav/auth-none/test.txt");
  assert(code == 204)
  assert(body == "")
end

function suite:test6()
  local code, header, body = client:request("GET", "https://brigid.jp/test/dav/auth-none/test.txt")
  assert(code == 404)
end

function suite:test7()
  local code, header, body = client:request("HEAD", "https://brigid.jp/test/dav/auth-none/test.txt")
  assert(code == 404)
  assert(body == "")
end

function suite:test8()
  local code, header, body = client:request_file("PUT", "https://brigid.jp/test/dav/auth-none/test.txt", nil, test_cwd .. "/test.dat")
  assert(code == 201 or code == 204)
end

function suite:test9()
  local code, header, body = client:request_file("GET", "https://brigid.jp/test/dav/auth-none/test.txt")
  assert(code == 200)
  assert(body == data4k:rep(1024 * 1024 / #data4k))
end

function suite:test10()
  local code, header, body = client:request_file("DELETE", "https://brigid.jp/test/dav/auth-none/test.txt")
  assert(code == 204)
  assert(body == "")
end

function suite:test11()
  local code, header, body = client:request("GET", "https://brigid.jp/test/cgi/redirect.cgi?count=1")
  assert(code == 200)
  assert(body == "ok\n")
  assert(not header.Locatuion)
end

function suite:test12()
  local code, header, body = client:request("GET", "https://brigid.jp/test/cgi/redirect.cgi?count=11")
  assert(code == 200)
  assert(body == "ok\n")
  assert(not header.Locatuion)
end

function suite:test13()
  local result, message = client:request("GET", "https://brigid.jp/test/cgi/redirect.cgi?count=22")
  print(message)
  assert(not result)
end

function suite:test14()
  local code, header, body = client:request("GET", "https://brigid.jp/test/dav/auth-basic/")
  assert(code == 401)
end

function suite:test_close_client_noauth()
  client:close()
  local result, message = pcall(function ()
    client:request("GET", "https://brigid.jp/")
  end)
  print(message)
  assert(not result)
  client = nil
end

function suite:test_open_client_auth()
  client = test_http_client("brigid", "O6jIOchrWCGuOSB4")
end

function suite:test15()
  local code, header, body = client:request("GET", "https://brigid.jp/test/dav/auth-basic/")
  assert(code == 200)
end

function suite:test_close_client_auth()
  client:close()
  client = nil
end

function suite:test_open_client()
  client = test_http_client()
end

local data = [[{"foo":42}]]
local request_header = { ["Content-Type"] = "application/json; charset=UTF-8" }

function suite:test16()
  local code, header, body = client:request_data("POST", "https://brigid.jp/test/cgi/cat.cgi", request_header, data)
  assert(code == 200)
  assert(body == data)
end

function suite:test17()
  local code, header, body = client:request_data("POST", "https://brigid.jp/test/cgi/env.cgi", request_header, data)
  assert(code == 200)
  assert(body:find("CONTENT_TYPE=application/json; charset=UTF-8\n", 1, true))
end

function suite:test18()
  local result, message = client:request("GET", "https://133.242.153.239/")
  print(message)
  assert(not result)
end

function suite:test19()
  local code, header, body = client:request("GET", "https://brigid.jp/test/cgi/nph-header.cgi")
  assert(code == 200)

  for k, v in pairs(header) do
    print("[" .. k .. "]=[" .. v .. "]")
  end

  assert(header["X-Test1"] == "foo bar")
  assert(header["X-Test2"] == "foo bar")
  assert(header["X-Test3"] == "foo  bar")
  assert(header["X-Test4"]:find "^foo[ \t]+bar$")
  assert(header["X-Test5"]:find "^foo[ \t]+bar$")
  assert(header["X-Test6"] == "foo bar")
  assert(header["x-test7"] == "foo bar")
  assert(header["X-tEsT8"] == "foo bar")
end

local data = ""
local request_header = { ["Content-Type"] = "application/x-www-form-urlencoded" }

function suite:test20()
  local code, header, body = client:request_data("POST", "https://brigid.jp/test/cgi/cat.cgi", request_header, data)
  assert(code == 200)
  assert(body == "")
end

function suite:test21()
  local code, header, body = client:request_data("POST", "https://brigid.jp/test/cgi/env.cgi", request_header, data)
  assert(code == 200)
  assert(body:find("CONTENT_TYPE=application/x-www-form-urlencoded\n", 1, true))
end

function suite:test22()
  local result, message = client:request("GET", "https://no-such-host.brigid.jp/")
  print(message)
  assert(not result)
end

function suite:test23()
  local result, message = client:request("GET", "!!! invalid url !!!")
  print(message)
  assert(not result)
end

function suite:test24()
  local code, header, body = client:request("POST", "https://brigid.jp/test/%63%67%69/env.cgi?%20%21")
  assert(code == 200)
  assert(body:find("QUERY_STRING=%20%21\n", 1, true))
end

function suite:test25()
  local code, header, body = client:request("GET", "https://brigid.jp/test/cgi/cookie.cgi")
  assert(code == 200)
  assert(header["Set-Cookie"] == "code=42; Path=/test/cgi/; Secure")
  assert(body == "HTTP_COOKIE=\n")
end

function suite:test26()
  local code, header, body = client:request("GET", "https://brigid.jp/test/cgi/cookie.cgi")
  assert(code == 200)
  assert(header["Set-Cookie"] == "code=42; Path=/test/cgi/; Secure")
  assert(body == "HTTP_COOKIE=\n")
end

function suite:test27()
  local code, header, body = client:request("GET", "https://brigid.jp/test/cgi/cookie.cgi", { Cookie = "code=69" })
  assert(code == 200)
  assert(not header["Set-Cookie"])
  assert(body == "HTTP_COOKIE=code=69\n")
end

function suite:test28()
  local ua = "brigid/" .. brigid.get_version()
  local code, header, body = client:request("GET", "https://brigid.jp/test/cgi/env.cgi", { ["User-Agent"] = ua })
  assert(code == 200)
  assert(body:find("USER_AGENT=" .. ua .. "\n", 1, true))
end

function suite:test_close_client()
  assert(client.session:close())
  assert(client.session:close())
  client = nil
end

function suite:test29()
  -- explicitly cancel in the progress callback
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
    file = test_cwd .. "/test.dat";
  }
  print(message, canceling)
  assert(not result)
  assert(message == "canceled")
  assert(canceling == 1)
end

function suite:test_remove_data()
  os.remove(test_cwd .. "/test.dat")
end

return suite
