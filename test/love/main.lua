-- Copyright (c) 2019 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local love = love
local t = {}

local ciphers = {
  "aes-128-cbc",
  "aes-192-cbc",
  "aes-256-cbc",
}
local plaintext = "The quick brown fox jumps over the lazy dog"
local keys = {
  ["aes-128-cbc"] = "0123456789012345";
  ["aes-192-cbc"] = "012345678901234567890123";
  ["aes-256-cbc"] = "01234567890123456789012345678901";
}
local iv = "0123456789012345"
local ciphertexts = {
  ["aes-128-cbc"] = "\048\137\230\188\034\075\217\091\133\207\086\244\185\103\017\138\170\071\005\067\015\037\182\180\217\083\024\138\209\093\215\143\056\103\087\126\125\088\225\140\156\179\064\100\124\139\079\216";
  ["aes-192-cbc"] = "\112\238\215\052\099\031\255\042\126\000\177\112\122\237\025\187\169\081\032\139\127\241\047\040\208\067\200\108\082\006\044\062\063\214\193\084\142\078\121\132\005\208\057\208\070\063\028\021";
  ["aes-256-cbc"] = "\224\111\099\167\017\232\183\170\159\148\064\016\125\070\128\161\023\153\067\128\234\049\210\162\153\185\083\002\212\057\185\112\044\142\101\169\146\054\236\146\007\004\145\092\241\169\138\068";
}

function love.load()
  local a, b = pcall(require, "brigid")

  if not a then
    t[#t + 1] = ("FAIL require %s\n"):format(b)
    return
  end

  local brigid = b

  for i = 1, #ciphers do
    local cipher = ciphers[i]
    local key = keys[cipher]
    local ciphertext = ciphertexts[cipher]
    local a, b = pcall(brigid.encrypt_string, cipher, plaintext, key, iv)
    if not a then
      t[#t + 1] = ("FAIL encrypt_string %s %s\n"):format(cipher, b)
    elseif b == ciphertext then
        t[#t + 1] = ("PASS encrypt_string %s\n"):format(cipher)
    else
      t[#t + 1] = ("FAIL encrypt_string %s\n"):format(cipher)
    end
    local a, b = pcall(brigid.decrypt_string, cipher, ciphertext, key, iv)
    if not a then
      t[#t + 1] = ("FAIL decrypt_string %s %s\n"):format(cipher, b)
    elseif b == plaintext then
        t[#t + 1] = ("PASS decrypt_string %s\n"):format(cipher)
    else
      t[#t + 1] = ("FAIL decrypt_string %s\n"):format(cipher)
    end
  end
end

function love.draw()
  local width = love.window.getMode()
  love.graphics.printf(table.concat(t), 50, 50, width - 100)
end
