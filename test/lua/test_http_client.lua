-- Copyright (c) 2021 <dev@brigid.jp>
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

return setmetatable(class, {
  __call = function (_, username, password)
    return setmetatable(class.new(username, password), metatable)
  end;
})
