-- Copyright (c) 2021 <dev@brigid.jp>
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
        url = "http://brigid.jp/pub/brigid-1.7-osx-x64.so";
        filename = "brigid.so";
        size = 188112;
        sha256 = "\131\154\155\192\054\202\204\160\126\107\138\067\061\018\024\224\094\044\028\155\186\077\007\166\099\062\047\117\093\150\092\201";
      };
    };
    Windows = {
      x64 = {
        url = "http://brigid.jp/pub/brigid-1.7-win-x64.dll";
        filename = "brigid.dll";
        size = 104960;
        sha256 = "\037\002\247\059\197\237\093\138\165\019\009\042\148\181\042\142\080\108\200\026\108\185\234\080\079\254\092\236\037\107\107\041";
      };
      x86 = {
        url = "http://brigid.jp/pub/brigid-1.7-win-x86.dll";
        filename = "brigid.dll";
        size = 83968;
        sha256 = "\071\135\203\041\174\051\145\005\246\037\159\101\224\142\166\242\138\174\150\016\038\130\062\094\013\129\132\006\208\154\035\185";
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
