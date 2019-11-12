local love = love
local t = {}

local expect = {
  0xE0, 0x6F, 0x63, 0xA7, 0x11, 0xE8, 0xB7, 0xAA, 0x9F, 0x94, 0x40, 0x10, 0x7D, 0x46, 0x80, 0xA1,
  0x17, 0x99, 0x43, 0x80, 0xEA, 0x31, 0xD2, 0xA2, 0x99, 0xB9, 0x53, 0x02, 0xD4, 0x39, 0xB9, 0x70,
  0x2C, 0x8E, 0x65, 0xA9, 0x92, 0x36, 0xEC, 0x92, 0x07, 0x04, 0x91, 0x5C, 0xF1, 0xA9, 0x8A, 0x44,
}

function love.load()
  t[#t + 1] = ("package.path = %s\n"):format(package.path)
  t[#t + 1] = ("package.cpath = %s\n"):format(package.cpath)

  local a, b = pcall(require, "brigid_core")

  if not a then
    t[#t + 1] = ("[FAIL] could not load module: %s\n"):format(b)
    return
  end

  local brigid_core = b

  local a, b = pcall(brigid_core.crypto.encrypt_string,
      "The quick brown fox jumps over the lazy dog",
      "01234567890123456789012345678901",
      "0123456789012345")

  if not a then
    t[#t + 1] = ("[FAIL] could not load encrypt_string: %s\n"):format(b)
    return
  end

  local result = b

  local check = #expect == #result
  for i = 1, #result do
    local byte = result:byte(i)
    t[#t + 1] = ("%02X"):format(byte)
    if i % 16 == 0 then
      t[#t + 1] = "\n"
    else
      t[#t + 1] = " "
    end
    if expect[i] ~= byte then
      check = false
    end
  end

  if check then
    t[#t + 1] = "check ok\n"
  else
    t[#t + 1] = "check error\n"
  end
end

function love.draw()
  local width = love.window.getMode()
  love.graphics.printf(table.concat(t), 50, 50, width - 100)
end
