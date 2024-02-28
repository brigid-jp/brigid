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
        url = "http://brigid.jp/pub/brigid-1.12-osx-x64.so";
        filename = "brigid.so";
        size = 293656;
        sha256 = "\158\142\218\044\222\203\244\116\029\192\036\132\169\131\214\250\133\195\228\128\202\234\147\028\045\024\124\087\035\017\060\167";
      };
    };
    Windows = {
      x64 = {
        url = "http://brigid.jp/pub/brigid-1.12-win-x64.dll";
        filename = "brigid.dll";
        size = 176640;
        sha256 = "\237\153\091\118\209\078\148\064\077\219\210\120\240\049\082\190\123\222\134\150\238\078\204\007\200\172\004\198\006\197\214\159";
      };
      x86 = {
        url = "http://brigid.jp/pub/brigid-1.12-win-x86.dll";
        filename = "brigid.dll";
        size = 142336;
        sha256 = "\252\229\250\148\122\150\166\129\078\107\049\035\243\220\043\129\010\241\013\150\020\214\012\206\013\202\247\097\043\198\047\144";
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
  assert(brigid.get_version() == "1.12")
  test_cwd = love.filesystem.getSaveDirectory()
  local suites = test_suites()
  suites()
end

local result, message = pcall(main)
if not result then
  print("error: " .. message)
end
