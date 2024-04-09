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
        url = "http://brigid.jp/pub/brigid-1.16-osx-x64.so";
        filename = "brigid.so";
        size = 270080;
        sha256 = "\157\144\117\086\109\153\139\158\051\051\073\038\158\137\158\137\054\204\225\070\155\213\190\078\201\103\178\238\099\116\184\204";
      };
    };
    Windows = {
      x64 = {
        url = "http://brigid.jp/pub/brigid-1.16-win-x64.dll";
        filename = "brigid.dll";
        size = 183808;
        sha256 = "\209\245\234\055\236\159\166\053\121\093\066\217\054\083\240\163\025\208\170\230\249\241\161\184\072\169\051\042\247\133\023\251";
      };
      x86 = {
        url = "http://brigid.jp/pub/brigid-1.16-win-x86.dll";
        filename = "brigid.dll";
        size = 147968;
        sha256 = "\178\151\055\181\226\161\054\008\074\093\077\034\186\138\063\017\222\205\207\009\004\135\206\098\063\128\206\138\083\066\223\191";
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
  assert(brigid.get_version() == "1.16")
  test_cwd = love.filesystem.getSaveDirectory()
  local suites = test_suites()
  suites()
end

local result, message = pcall(main)
if not result then
  print("error: " .. message)
end
