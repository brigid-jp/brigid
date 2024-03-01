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
        url = "http://brigid.jp/pub/brigid-1.13-osx-x64.so";
        filename = "brigid.so";
        size = 293720;
        sha256 = "\251\006\046\007\071\118\003\063\067\190\077\155\246\066\142\185\114\024\211\112\213\139\097\170\012\185\052\216\022\088\166\023";
      };
    };
    Windows = {
      x64 = {
        url = "http://brigid.jp/pub/brigid-1.13-win-x64.dll";
        filename = "brigid.dll";
        size = 177152;
        sha256 = "\206\235\156\157\190\032\147\040\163\231\232\040\231\155\218\047\243\002\167\156\155\091\097\059\151\086\144\111\128\167\136\184";
      };
      x86 = {
        url = "http://brigid.jp/pub/brigid-1.13-win-x86.dll";
        filename = "brigid.dll";
        size = 142848;
        sha256 = "\074\002\018\094\211\250\151\092\152\005\187\096\137\199\061\121\244\128\164\146\026\171\145\036\238\003\143\162\244\029\099\004";
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
  assert(brigid.get_version() == "1.13")
  test_cwd = love.filesystem.getSaveDirectory()
  local suites = test_suites()
  suites()
end

local result, message = pcall(main)
if not result then
  print("error: " .. message)
end
