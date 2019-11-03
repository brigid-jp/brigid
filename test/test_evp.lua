local brigid_core = require "brigid_core"

if not brigid_core.evp then
  os.exit()
end

print(brigid_core.evp)
print(brigid_core.evp.encrypt_string)

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
