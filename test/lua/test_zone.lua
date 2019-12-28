-- Copyright (c) 2019 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local brigid = require "brigid"

local password = table.concat {
  "\046\110\216\048\063\098\218\044";
  "\235\128\153\113\148\171\195\042";
  "\182\087\137\084\207\110\072\076";
  "\147\008\056\214\030\104\215\147";
}

-- io.write(password)
-- os.exit()

local sha256 = table.concat {
  "\183\068\145\017\168\130\179\105";
  "\080\036\178\241\053\137\078\236";
  "\001\123\084\124\035\135\057\014";
  "\061\203\248\075\152\252\066\019";
}

local sha512 = table.concat {
  "\002\090\052\198\136\230\135\147";
  "\163\202\192\095\046\182\072\025";
  "\096\014\090\173\020\110\195\120";
  "\213\045\222\195\116\049\058\163";
  "\157\012\091\023\082\030\083\039";
  "\139\098\225\170\225\226\070\185";
  "\244\002\099\142\017\151\098\199";
  "\110\121\182\216\253\121\244\054";
}

local ciphertext_128_256 = table.concat {
  "\083\097\108\116\101\100\095\095";
  "\166\232\215\233\042\197\245\164";
  "\017\053\230\029\194\120\098\034";
  "\168\186\047\114\120\046\142\099";
  "\160\238\249\117\121\228\159\074";
  "\175\174\244\035\179\228\009\144";
  "\165\195\197\054\137\089\227\093";
  "\123\109\127\036\063\068\028\104";
}

local ciphertext_192_256 = table.concat {
  "\083\097\108\116\101\100\095\095";
  "\086\004\091\014\047\015\186\189";
  "\192\255\114\139\018\143\132\138";
  "\045\124\092\031\193\163\065\131";
  "\158\062\015\035\136\059\076\206";
  "\136\154\120\053\062\195\120\134";
  "\070\205\148\178\150\253\169\241";
  "\215\065\093\031\043\016\142\111";
}

local ciphertext_256_256 = table.concat {
  "\083\097\108\116\101\100\095\095";
  "\002\143\161\095\097\170\128\069";
  "\011\064\074\007\108\043\211\062";
  "\138\224\020\015\022\216\072\189";
  "\119\112\183\110\093\079\201\019";
  "\115\114\143\252\002\165\195\063";
  "\092\149\186\127\026\164\251\253";
  "\255\214\177\150\123\183\136\042";
}

local ciphertext_256_512 = table.concat {
  "\083\097\108\116\101\100\095\095";
  "\185\188\009\019\054\116\055\214";
  "\101\147\033\158\054\036\036\178";
  "\036\006\146\158\077\167\250\068";
  "\214\113\239\163\150\148\034\114";
  "\136\013\041\097\176\007\201\013";
  "\059\164\118\143\107\238\098\124";
  "\060\005\144\201\241\201\157\094";
}

local ciphertext_chunk = table.concat {
  "\083\097\108\116\101\100\095\095";
  "\097\141\000\181\099\092\002\216";
  "\113\072\165\134\134\209\119\155";
  "\023\144\149\110\066\225\064\162";
}

local sha256_chunk = table.concat {
  "\225\021\201\185\149\247\064\024";
  "\099\107\255\017\031\101\237\177";
  "\125\200\134\007\152\122\249\071";
  "\224\085\176\115\096\151\255\110";
}

local function check(cipher, hash, ciphertext)
  assert(ciphertext:sub(1, 8) == "Salted__")
  local decryptor = brigid.zone.decryptor(cipher, hash, ciphertext:sub(9, 16), function (out)
    assert(out:get_string() == "The quick brown fox jumps over the lazy dog")
  end)
  decryptor:update(ciphertext:sub(17), true)
end

local result = brigid.zone.sha256()
for i = 1, #result do
  io.write(("%02X"):format(result:byte(i)))
end
io.write "\n"

for i = 1, 32 do
  brigid.zone.put(i, password:byte(i))
end

assert(brigid.zone.sha256() == sha256)
assert(brigid.zone.sha512() == sha512)

check("aes-128-cbc", "sha256", ciphertext_128_256)
check("aes-192-cbc", "sha256", ciphertext_192_256)
check("aes-256-cbc", "sha256", ciphertext_256_256)
check("aes-256-cbc", "sha512", ciphertext_256_512)

local chunk = assert(brigid.zone.load("aes-256-cbc", "sha256", ciphertext_chunk))
assert(chunk() == 42)

local chunk = assert(brigid.zone.load("aes-256-cbc", "sha256", ciphertext_chunk, sha256_chunk))
assert(chunk() == 42)

local result, message = brigid.zone.load("aes-256-cbc", "sha256", ciphertext_chunk, ("0"):rep(32))
print(message)
assert(not result)
