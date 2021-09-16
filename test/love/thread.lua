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
    -- ensure save directory
    love.filesystem.write("dummy.dat", os.date "%Y-%m-%d %H:%M:%S")
    send("real directory: ", love.filesystem.getRealDirectory "dummy.dat")
    love.filesystem.remove "dummy.dat"
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
            -- send("progress ", now, " / ", module_info.size)
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
  assert(data)
  assert(type(data) == "string")
  assert(size == 12)

  local ptr = decode_pointer(data)
  assert(ptr)
  assert(type(ptr) == "cdata")

  local ffi_ptr = byte_data:getFFIPointer()
  assert(ptr == ffi_ptr)

  local ffi_data = encode_pointer(ffi_ptr)
  assert(data == ffi_data)
end

local plaintext = "The quick brown fox jumps over the lazy dog"
local ciphers = {
  "aes-128-cbc",
  "aes-192-cbc",
  "aes-256-cbc",
}
local keys = {
  ["aes-128-cbc"] = "0123456789012345";
  ["aes-192-cbc"] = "012345678901234567890123";
  ["aes-256-cbc"] = "01234567890123456789012345678901";
}
local iv = "0123456789012345"
local ciphertexts = {
  ["aes-128-cbc"] = table.concat {
    "\048\137\230\188\034\075\217\091";
    "\133\207\086\244\185\103\017\138";
    "\170\071\005\067\015\037\182\180";
    "\217\083\024\138\209\093\215\143";
    "\056\103\087\126\125\088\225\140";
    "\156\179\064\100\124\139\079\216";
  };
  ["aes-192-cbc"] = table.concat {
    "\112\238\215\052\099\031\255\042";
    "\126\000\177\112\122\237\025\187";
    "\169\081\032\139\127\241\047\040";
    "\208\067\200\108\082\006\044\062";
    "\063\214\193\084\142\078\121\132";
    "\005\208\057\208\070\063\028\021";
  };
  ["aes-256-cbc"] = table.concat {
    "\224\111\099\167\017\232\183\170";
    "\159\148\064\016\125\070\128\161";
    "\023\153\067\128\234\049\210\162";
    "\153\185\083\002\212\057\185\112";
    "\044\142\101\169\146\054\236\146";
    "\007\004\145\092\241\169\138\068";
  };
}

local function encrypt(cipher, key, iv, plaintext)
  local result = {}
  local cryptor = assert(brigid.encryptor(cipher, key, iv, function (view)
    result[#result + 1] = view:get_string()
  end))
  assert(cryptor:update(plaintext, true))
  return table.concat(result)
end

local function decrypt(cipher, key, iv, ciphertext)
  local result = {}
  local cryptor = assert(brigid.decryptor(cipher, key, iv, function (view)
    result[#result + 1] = view:get_string()
  end))
  assert(cryptor:update(ciphertext, true))
  return table.concat(result)
end

for i = 1, #ciphers do
  local cipher = ciphers[i]
  local key = keys[cipher]
  local ciphertext = ciphertexts[cipher]
  local cipher_name = cipher:gsub("%-", "_")
  test_cases["test_crypto_" .. cipher_name .. "_encrypt"] = function ()
    assert(encrypt(cipher, key, iv, plaintext) == ciphertext)
  end
  test_cases["test_crypto_" .. cipher_name .. "_decrypt"] = function ()
    assert(decrypt(cipher, key, iv, ciphertext) == plaintext)
  end
end

function test_cases.test_crpto_sha256_1()
  local result = brigid.hasher "sha256":update "":digest()
  assert(result == table.concat {
    "\227\176\196\066\152\252\028\020";
    "\154\251\244\200\153\111\185\036";
    "\039\174\065\228\100\155\147\076";
    "\164\149\153\027\120\082\184\085";
  })
end

function test_cases.test_crpto_sha256_2()
  local result = brigid.hasher "sha256":update "The quick brown fox jumps over the lazy dog":digest()
  assert(result == table.concat {
    "\215\168\251\179\007\215\128\148";
    "\105\202\154\188\176\008\046\079";
    "\141\086\081\228\109\060\219\118";
    "\045\002\208\191\055\201\229\146";
  })
end

function test_cases.test_crpto_sha512_1()
  local result = brigid.hasher "sha512":update "":digest()
  assert(result == table.concat {
    "\207\131\225\053\126\239\184\189";
    "\241\084\040\080\214\109\128\007";
    "\214\032\228\005\011\087\021\220";
    "\131\244\169\033\211\108\233\206";
    "\071\208\209\060\093\133\242\176";
    "\255\131\024\210\135\126\236\047";
    "\099\185\049\189\071\065\122\129";
    "\165\056\050\122\249\039\218\062";
  })
end

function test_cases.test_crpto_sha512_2()
  local result = brigid.hasher "sha512":update "The quick brown fox jumps over the lazy dog":digest()
  assert(result == table.concat {
    "\007\229\071\217\088\111\106\115";
    "\247\063\186\192\067\094\215\105";
    "\081\033\143\183\208\200\215\136";
    "\163\009\215\133\067\107\187\100";
    "\046\147\162\082\169\084\242\057";
    "\018\084\125\030\138\059\094\214";
    "\225\191\215\009\120\033\035\063";
    "\160\083\143\061\184\084\254\230";
  })
end

function test_cases.test_version()
  local version = brigid.get_version()
  send("version: ", version)
  assert(version:match "^%d+%.%d+$")
end

function test_cases.test_writer1()
  local writer = assert(brigid.data_writer())
  assert(writer:write "foo\n")
  assert(writer:write "bar\n")
  assert(writer:write "baz\n")
  assert(writer:write "qux\n")
  assert(writer:get_string() == "foo\nbar\nbaz\nqux\n")
  assert(writer:get_size() == 16)
  assert(writer:close())
  local result, message = pcall(function () writer:get_size() end)
  send("message: ", message)
  assert(not result)
end

function test_cases.test_writer2()
  local filename = love.filesystem.getSaveDirectory().. "/test.dat"
  send("filename: ", filename)

  local writer = assert(brigid.file_writer(filename))
  assert(writer:write "foo\n")
  assert(writer:write "bar\n")
  assert(writer:write "baz\n")
  assert(writer:write "qux\n")
  assert(writer:close())

  local handle = assert(io.open(filename))
  assert(handle:read "*a" == "foo\nbar\nbaz\nqux\n")
  handle:close()
  os.remove(filename)

  local writer, message = brigid.file_writer "no such directory/test.dat"
  send("message: ", message)
  assert(not writer)
end

function test_cases.test_writer3()
  local writer = assert(brigid.data_writer())
  assert(writer:write "foo\n")
  assert(writer:write "bar\n")
  assert(writer:write(love.data.newByteData "baz\n"))
  assert(writer:write "qux\n")
  assert(writer:get_string() == "foo\nbar\nbaz\nqux\n")
  assert(writer:get_size() == 16)
  assert(writer:close())
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
      send("[FAIL] ", test_case_name, " ", message)
    end
  end

  send("TOTAL: ", #test_case_names)
  send("PASS:  ", test_count_pass)
  send("FAIL:  ", test_count_fail)
end

send "thread started"
local result, message = pcall(main)
if not result then
  send("ERROR: ", message)
end
send "thread finished"
