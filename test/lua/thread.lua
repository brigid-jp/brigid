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
        url = "http://brigid.jp/pub/brigid-1.15-osx-x64.so";
        filename = "brigid.so";
        size = 270080;
        sha256 = "\063\016\076\159\001\144\106\085\225\202\091\056\091\236\145\064\100\142\125\131\164\212\231\175\216\179\034\250\143\131\117\202";
      };
    };
    Windows = {
      x64 = {
        url = "http://brigid.jp/pub/brigid-1.15-win-x64.dll";
        filename = "brigid.dll";
        size = 183808;
        sha256 = "\174\250\021\242\034\238\011\149\029\250\170\052\218\226\065\144\143\057\161\217\042\166\195\246\123\101\095\176\184\074\146\037";
      };
      x86 = {
        url = "http://brigid.jp/pub/brigid-1.15-win-x86.dll";
        filename = "brigid.dll";
        size = 147968;
        sha256 = "\093\223\170\168\146\154\150\090\121\163\037\210\205\069\167\055\049\139\184\214\095\168\115\006\245\058\085\141\067\066\138\047";
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
  assert(brigid.get_version() == "1.15")
  test_cwd = love.filesystem.getSaveDirectory()
  local suites = test_suites()
  suites()
end

local result, message = pcall(main)
if not result then
  print("error: " .. message)
end
