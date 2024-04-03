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
        url = "http://brigid.jp/pub/brigid-1.14-osx-x64.so";
        filename = "brigid.so";
        size = 270136;
        sha256 = "\210\061\151\206\242\101\247\144\104\224\196\092\091\174\098\037\047\101\128\117\152\027\075\188\017\252\189\163\152\163\197\183";
      };
    };
    Windows = {
      x64 = {
        url = "http://brigid.jp/pub/brigid-1.14-win-x64.dll";
        filename = "brigid.dll";
        size = 184320;
        sha256 = "\060\047\251\011\128\220\016\014\206\035\000\055\165\047\247\220\139\168\047\131\023\222\025\033\065\130\011\114\254\117\132\097";
      };
      x86 = {
        url = "http://brigid.jp/pub/brigid-1.14-win-x86.dll";
        filename = "brigid.dll";
        size = 147968;
        sha256 = "\074\007\203\005\187\011\203\218\224\108\145\137\165\189\190\092\190\203\218\072\002\255\239\191\151\154\172\041\234\092\033\239";
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
  assert(brigid.get_version() == "1.14")
  test_cwd = love.filesystem.getSaveDirectory()
  local suites = test_suites()
  suites()
end

local result, message = pcall(main)
if not result then
  print("error: " .. message)
end
