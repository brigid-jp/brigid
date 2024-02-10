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
        url = "http://brigid.jp/pub/brigid-1.9-osx-x64.so";
        filename = "brigid.so";
        size = 271072;
        sha256 = "\042\019\161\212\177\211\199\205\065\223\143\234\017\122\209\206\189\128\233\227\030\233\158\082\169\176\201\066\059\144\019\090";
      };
    };
    Windows = {
      x64 = {
        url = "http://brigid.jp/pub/brigid-1.9-win-x64.dll";
        filename = "brigid.dll";
        size = 156672;
        sha256 = "\178\120\198\033\201\150\243\078\137\062\200\031\240\031\150\204\236\209\253\019\197\163\133\140\162\132\231\022\035\201\224\116";
      };
      x86 = {
        url = "http://brigid.jp/pub/brigid-1.9-win-x86.dll";
        filename = "brigid.dll";
        size = 128512;
        sha256 = "\233\025\208\114\124\207\213\228\064\193\186\048\206\171\243\103\254\190\225\108\181\043\045\163\056\167\148\133\198\198\037\014";
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
