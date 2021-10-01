#! /usr/bin/env lua

local class = {}
local metatable = { __index = class }

function class:rulename()
  local v = self.data:match "^([A-Za-z][A-Za-z0-9%-]*)"
  if v then
    print("rulename", v)
  else
    error "cannot match"
  end
end

function class:any_c_wsp()
  while self:c_wsp() do end
end

function class:defined_as()
  self:any_c_wsp()
  -- "=" or "=/"
  -- defined_as op
  self:any_c_wsp()
end

--[[
local path = ...

local handle = assert(io.open(path))
local content = handle:read "*a"
handle:close()

-- content = content:gsub("\n[^\n]+%[Page %d+%]\n\f\n[^\n]+\n", "\n")

content = content
  :gsub("\n[^\n]*\n\f\n", "\n\f\n")
  :gsub("\f\n[^\n]*\n", "\f\n")
  :gsub("\n*\f\n*", "\n\n")
  :gsub("\n\n+", "\n\n")

io.write(content)
]]

--[[

for line in io.lines(path) do
  local state = 1

  if state == 1 and line:find "^Appendix.*Collected ABNF" then
    print(line)
  elseif state == 2 then
  end

end
]]
