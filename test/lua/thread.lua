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
        url = "http://brigid.jp/pub/brigid-1.6-osx-x64.so";
        filename = "brigid.so";
        size = 187552;
        sha256 = "\138\188\134\099\117\095\026\079\154\232\084\204\184\048\096\236\117\174\233\125\224\010\105\134\189\028\128\151\108\126\247\211";
      };
    };
    Windows = {
      x64 = {
        url = "http://brigid.jp/pub/brigid-1.6-win-x64.dll";
        filename = "brigid.dll";
        size = 104960;
        sha256 = "\139\190\032\202\006\061\024\077\149\038\126\050\047\085\088\109\226\232\030\089\116\113\118\220\222\019\030\222\163\047\147\222";
      };
      x86 = {
        url = "http://brigid.jp/pub/brigid-1.6-win-x86.dll";
        filename = "brigid.dll";
        size = 83968;
        sha256 = "\254\069\138\082\234\055\210\093\089\092\126\166\157\156\162\112\224\145\223\026\062\054\221\058\066\193\161\062\230\146\231\155";
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
