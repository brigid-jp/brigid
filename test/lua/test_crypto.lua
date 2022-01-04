-- Copyright (c) 2019,2021,2022 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local brigid = require "brigid"
local test_suite = require "test_suite"

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

local suite = test_suite "test_crypto"

local cipher = "aes-256-cbc"
local key = keys[cipher]
local ciphertext = ciphertexts[cipher]

function suite:test_cryptor()
  local cryptor = assert(brigid.encryptor(cipher, key, iv))
  assert(cryptor:update(plaintext, true))
  assert(cryptor:close())
  assert(cryptor:close()) -- can close
  local result, message = pcall(function () cryptor:update "0" end)
  print(message)
  assert(not result)
end

for i = 1, #ciphers do
  local cipher = ciphers[i]
  local key = keys[cipher]
  local ciphertext = ciphertexts[cipher]
  local cipher_name = cipher:gsub("%-", "_")
  suite["test_encrypt_" .. cipher_name] = function ()
    assert(encrypt(cipher, key, iv, plaintext) == ciphertext)
  end
  suite["test_decrypt_" .. cipher_name] = function ()
    assert(decrypt(cipher, key, iv, ciphertext) == plaintext)
  end
end

function suite:test_sha1_1()
  local result = brigid.hasher "sha1":update "":digest()
  assert(result == table.concat {
    "\218\057\163\238\094\107\075\013";
    "\050\085\191\239\149\096\024\144";
    "\175\216\007\009"
  })
end

function suite:test_sha1_2()
  local result = brigid.hasher "sha1":update "The quick brown fox jumps over the lazy dog":digest()
  assert(result == table.concat {
    "\047\212\225\198\122\045\040\252";
    "\237\132\158\225\187\118\231\057";
    "\027\147\235\018"
  })
end

function suite:test_sha1_3()
  local h = brigid.hasher "sha1"
  if debug then print(h:get_name()) end
  assert(h:get_name() == "sha1")
end

function suite:test_sha256_1()
  local result = brigid.hasher "sha256":update "":digest()
  assert(result == table.concat {
    "\227\176\196\066\152\252\028\020";
    "\154\251\244\200\153\111\185\036";
    "\039\174\065\228\100\155\147\076";
    "\164\149\153\027\120\082\184\085";
  })
end

function suite:test_sha256_2()
  local result = brigid.hasher "sha256":update "The quick brown fox jumps over the lazy dog":digest()
  assert(result == table.concat {
    "\215\168\251\179\007\215\128\148";
    "\105\202\154\188\176\008\046\079";
    "\141\086\081\228\109\060\219\118";
    "\045\002\208\191\055\201\229\146";
  })
end

function suite:test_sha256_3()
  local h = brigid.hasher "sha256"
  if debug then print(h:get_name()) end
  assert(h:get_name() == "sha256")
end

function suite:test_sha512_1()
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

function suite:test_sha512_2()
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

function suite:test_sha512_3()
  local h = brigid.hasher "sha512"
  if debug then print(h:get_name()) end
  assert(h:get_name() == "sha512")
end

return suite
