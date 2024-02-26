-- Copyright (c) 2021,2024 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local channel = ...

local saver = {
  print = print;
}

function print(...)
  local buffer = {}
  for i = 1, select("#", ...) do
    buffer[i] = tostring(select(i, ...))
  end
  channel:push(table.concat(buffer, "\t"))
end

assert(love.thread)
love.data = require "love.data"
love.filesystem = require "love.filesystem"
love.system = require "love.system"

assert(not socket)
socket = {
  http = require "socket.http";
}

local brigid
local test_suites = require "test_suites"

local function boot()
  -- ensure save directory
  love.filesystem.write("dummy.dat", "")
  love.filesystem.remove "dummy.dat"

  local result, message = pcall(function ()
    brigid = require "brigid"
  end)
  if result then
    return
  end
  print("could not require brigid: " .. message)

  local module_informations = {
    ["OS X"] = {
      x64 = {
        url = "http://brigid.jp/pub/brigid-1.11-osx-x64.so";
        filename = "brigid.so";
        size = 276168;
        sha256 = "\097\015\104\230\198\130\103\196\217\078\045\255\165\048\157\245\187\015\040\222\046\166\082\023\227\034\105\041\133\137\079\211";
      };
    };
    Windows = {
      x64 = {
        url = "http://brigid.jp/pub/brigid-1.11-win-x64.dll";
        filename = "brigid.dll";
        size = 172544;
        sha256 = "\243\242\226\247\249\202\144\254\051\091\056\113\215\170\004\051\052\091\070\169\181\211\242\170\112\074\074\049\171\066\177\153";
      };
      x86 = {
        url = "http://brigid.jp/pub/brigid-1.11-win-x86.dll";
        filename = "brigid.dll";
        size = 139776;
        sha256 = "\124\131\153\043\240\051\203\111\074\145\063\006\233\042\017\192\008\122\152\255\242\071\022\032\189\249\014\086\157\123\185\217";
      };
    };
  }

  local system = module_informations[love.system.getOS()]
  if system then
    local module_info = system[jit.arch]
    if module_info then
      local now = 0
      assert(socket.http.request {
        url = module_info.url;
        sink = function (chunk, e)
          if chunk then
            if now == 0 then
              love.filesystem.write(module_info.filename, chunk)
            else
              love.filesystem.append(module_info.filename, chunk)
            end
            now = now + #chunk
            return true
          elseif e then
            error(e)
          end
        end;
      })

      local file_info = love.filesystem.getInfo(module_info.filename)
      assert(module_info.size == file_info.size)

      local file_sha256 = love.data.hash("sha256", assert(love.filesystem.newFileData(module_info.filename)))
      assert(module_info.sha256 == file_sha256)

      brigid = require "brigid"
    end
  end
end

local function main()
  boot()
  test_cwd = love.filesystem.getSaveDirectory()
  local suites = test_suites()
  suites()
end

local result, message = pcall(main)
if not result then
  print("error: " .. message)
end
