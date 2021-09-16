-- Copyright (c) 2021 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local love = {
  data = require "love.data";
  filesystem = require "love.filesystem";
  system = require "love.system";
}
local socket = {
  http = require "socket.http";
}
local brigid

local send_channel, recv_channel, intr_channel = ...

local function send(...)
  return send_channel:push(table.concat {...})
end

local function boot()
  local result, message = pcall(function ()
    brigid = require "brigid"
  end)
  if result then
    return
  end
  send("could not require brigid: ", message)

  local module_informations = {
    ["OS X"] = {
      x64 = {
        url = "http://brigid.jp/pub/brigid-1.5-osx-x64.so";
        filename = "brigid.so";
        size = 187168;
        sha256 = "\068\212\186\137\114\050\065\214\000\252\166\096\101\018\192\157\131\222\229\049\072\162\226\020\057\219\071\242\201\207\042\059";
      };
    };
    Windows = {
      x64 = {
        url = "http://brigid.jp/pub/brigid-1.5-win-x64.dll";
        filename = "brigid.dll";
        size = 103936;
        sha256 = "\070\209\046\163\058\157\181\171\251\081\169\108\009\139\028\080\164\218\062\207\042\219\240\255\220\214\117\082\046\080\131\080";
      };
      x86 = {
        url = "http://brigid.jp/pub/brigid-1.5-win-x86.dll";
        filename = "brigid.dll";
        size = 82432;
        sha256 = "\004\014\098\069\164\219\112\208\177\228\064\091\041\154\008\021\191\091\238\043\046\197\033\129\212\211\089\096\118\033\014\248";
      };
    };
  }

  local os = love.system.getOS()
  send("os: ", os)
  local system = module_informations[os]
  if system then
    local arch = jit.arch
    send("arch: ", arch)
    local module_info = system[arch]
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
            send("progress ", now, " / ", module_info.size)
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

local test_cases = {}

function test_cases.test_common()
  local registry = assert(debug.getregistry())
  local encode_pointer = assert(registry["brigid.common.encode_pointer"])
  local decode_pointer = assert(registry["brigid.common.decode_pointer"])
  local is_love2d_data = assert(registry["brigid.common.is_love2d_data"])

  local byte_data = love.data.newByteData "foobarbazqux"
  local data, size = is_love2d_data(byte_data)
  send(type(data))
  send(tostring(decode_pointer(data)))
  assert(data ~= nil)
  assert(size == 12)

  local ptr = byte_data:getFFIPointer()
  send(tostring(ptr))
  -- asert(data == ptr)
end

local test_case_names = {}
for test_case_name in pairs(test_cases) do
  test_case_names[#test_case_names + 1] = test_case_name
end
table.sort(test_case_names)

local function main()
  boot()

  local test_count_pass = 0
  local test_count_fail = 0

  for i = 1, #test_case_names do
    local test_case_name = test_case_names[i]
    local test_case = test_cases[test_case_name]

    local result, message = pcall(test_case)
    if result then
      test_count_pass = test_count_pass + 1
      send("[PASS] ", test_case_name)
    else
      test_count_fail = test_count_fail + 1
      send("[FAIL] ", test_case_name)
    end
  end

  send("TOTAL: ", #test_case_names)
  send("PASS:  ", test_count_pass)
  send("FAIL:  ", test_count_fail)
end

send "thread opened"
local result, message = pcall(main)
if not result then
  send("error: ", message)
end
send "thread closed"
