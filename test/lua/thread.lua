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
        url = "http://brigid.jp/pub/brigid-1.10-osx-x64.so";
        filename = "brigid.so";
        size = 274584;
        sha256 = "\183\023\096\233\162\088\135\013\167\151\228\244\108\048\107\242\225\028\220\004\211\052\201\250\115\148\136\116\045\000\181\206";
      };
    };
    Windows = {
      x64 = {
        url = "http://brigid.jp/pub/brigid-1.10-win-x64.dll";
        filename = "brigid.dll";
        size = 166912;
        sha256 = "\150\191\068\231\218\135\076\172\179\006\172\046\225\222\165\149\084\113\122\003\043\130\158\236\235\194\209\205\186\089\003\080";
      };
      x86 = {
        url = "http://brigid.jp/pub/brigid-1.10-win-x86.dll";
        filename = "brigid.dll";
        size = 135680;
        sha256 = "\070\115\198\175\045\022\138\034\173\125\084\194\009\064\173\176\223\152\136\170\167\212\163\024\169\046\085\148\016\073\064\221";
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
