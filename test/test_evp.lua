local brigid_core = require "brigid_core"

if not brigid_core.evp then
  os.exit()
end

print(brigid_core.evp)
print(brigid_core.evp.encrypt_string)

brigid_core.evp.encrypt_string("", "", "")
