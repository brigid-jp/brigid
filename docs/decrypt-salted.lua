-- Copyright (c) 2019 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local sha256 = ...
local brigid = require "brigid"

local password = "password"

assert(io.read(8) == "Salted__")
local salt = io.read(8)

local key = sha256(password .. salt)
local iv = sha256(key .. password .. salt):sub(1, 16)

local plaintext
local decryptor = brigid.decryptor("aes-256-cbc", key, iv, function (out)
  plaintext = out:get_string()
end)
decryptor:update(io.read "*a", true)

assert(plaintext == "The quick brown fox jumps over the lazy dog")
