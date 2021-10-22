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
        url = "http://brigid.jp/pub/brigid-1.8-osx-x64.so";
        filename = "brigid.so";
        size = 224032;
        sha256 = "\051\034\254\139\059\112\110\176\091\253\103\146\126\217\147\210\176\026\162\240\091\204\223\118\199\246\250\229\148\048\202\124";
      };
    };
    Windows = {
      x64 = {
        url = "http://brigid.jp/pub/brigid-1.8-win-x64.dll";
        filename = "brigid.dll";
        size = 129536;
        sha256 = "\145\016\183\032\031\008\044\075\065\069\065\100\189\103\074\009\126\007\241\063\021\071\003\190\095\136\039\062\049\198\100\017";
      };
      x86 = {
        url = "http://brigid.jp/pub/brigid-1.8-win-x86.dll";
        filename = "brigid.dll";
        size = 105984;
        sha256 = "\055\152\198\247\233\194\095\165\036\213\185\171\076\064\226\022\010\082\200\219\084\065\151\229\212\043\072\041\081\201\161\078";
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
