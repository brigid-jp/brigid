local brigid_core = require "brigid_core"

if not brigid_core.ccrypt then
  os.exit()
end

print(brigid_core.ccrypt)
print(brigid_core.ccrypt.encrypt_string)

os.exit()

-- https://wiki.openssl.org/index.php/EVP_Symmetric_Encryption_and_Decryption
local result = brigid_core.evp.encrypt_string(
    "The quick brown fox jumps over the lazy dog",
    "01234567890123456789012345678901",
    "0123456789012345")

for i = 1, #result do
  local byte = result:byte(i)
  io.write(("%02X"):format(byte))
  if i % 16 == 0 then
    io.write "\n"
  else
    io.write " "
  end
end

local expect_data = {
  0xE0, 0x6F, 0x63, 0xA7, 0x11, 0xE8, 0xB7, 0xAA, 0x9F, 0x94, 0x40, 0x10, 0x7D, 0x46, 0x80, 0xA1,
  0x17, 0x99, 0x43, 0x80, 0xEA, 0x31, 0xD2, 0xA2, 0x99, 0xB9, 0x53, 0x02, 0xD4, 0x39, 0xB9, 0x70,
  0x2C, 0x8E, 0x65, 0xA9, 0x92, 0x36, 0xEC, 0x92, 0x07, 0x04, 0x91, 0x5C, 0xF1, 0xA9, 0x8A, 0x44,
}
local expect = {}
for i = 1, #expect_data do
  expect[i] = string.char(expect_data[i])
end
assert(result, table.concat(expect))
