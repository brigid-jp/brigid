#! /usr/bin/env lua

local class = {}
local metatable = { __index = class }

local function new(name, ...)
  return { [0] = name, ... }
end

function class:push(node)
  self[#self + 1] = node
end

local abnf_node = setmetatable(class, {
  __call = function (_, name, ...)
    return setmetatable(new(name, ...), metatable)
  end;
})

local class = {}
local metatable = {}

local function new(buffer)
  return {
    buffer = buffer;
    position = 1;
    stack = {};
    matches = {};
  }
end

local function match(self, i, j, ...)
  if i then
    self.position = j + 1
    self.matches = { [0] = self.buffer:sub(i, j), ... }
    return true
  else
    return false
  end
end

function class:match(pattern)
  return match(self, self.buffer:find("^" .. pattern, self.position))
end

function class:top()
  local stack = self.stack
  return stack[#stack]
end

function class:push(node)
  local stack = self.stack
  stack[#stack + 1] = node
  assert(node[0])
end

function class:pop()
  local stack = self.stack
  local n = #stack
  local node = stack[n]
  stack[n] = nil
  return node
end

function class:backup()
  local stack = {}
  for k, v in pairs(self.stack) do
    stack[k] = v
  end
  return {
    position = self.position;
    stack = stack;
  }
end

function class:restore(backup)
  self.position = backup.position
  self.stack = backup.stack
end

function class:error()
  error("parser error: at position " .. self.position .. " source [" .. self.buffer:sub(self.position, self.position + 10) .. "]")
end

function rulelist(self)
  local backup = self:backup()

  if self:rule() then
    local rule = self:pop()
    self:top():push(rule)
    return true
  else
    while self:c_wsp() do end
    if self:c_nl() then
      return true
    end
  end

  self:restore(backup)
end

function class:rulelist()
  self:push(abnf_node("rulelist"))

  if not rulelist(self) then
    self:error()
  end

  while true do
    if not rulelist(self) then
      break
    end
  end

  -- 全部読んだかをチェックする
  -- トップにrulelistがあるかをチェックする
end

function class:rule()
  local backup = self:backup()

  if self:rulename() and self:defined_as() and self:elements() and self:c_nl() then
    -- rule nodeを作る
    return true
  end

  self:restore(backup)
end

function class:rulename()
  if self:match "%a[%a%d%-]*" then
    self:push(abnf_node("rulename", self[0]))
    return true
  end
end

function class:defined_as()
  local backup = self:backup()

  while self:c_wsp() do end
  if self:match "=/?" then
    self:push(abnf_node("defined_as", self[0]))
    while self:c_wsp() do end
    return true
  end

  self:restore(backup)
end

function class:elements()
  if self:alternation() then
    while self:c_wsp() do end
    local node = abnf_node("elements", self:pop())
    self:push(node)
    return true
  end
end

function class:c_wsp()
  local backup = self:backup()

  if self:match "[ \t]" then
    return true
  else
    if self:c_nl() then
      if self:match "[ \t]" then
        return true
      end
    end
  end

  self:restore(backup)
end

function class:c_nl()
  if self:comment() then
    return true
  elseif self:match "\n" then
    return true
  end
end

function class:comment()
  if self:match ";[ \t\33-\126]*\n" then
    return true
  end
end

function class:alternation()
  local backup = self:backup()

  if self:concatenation() then
    local node = abnf_node("alternation", self:pop())
    while true do
      local backup = self:backup()
      local commit
      while self:c_wsp() do end
      if self:match "/" then
        while self:c_wsp() do end
        if self:concatenation() then
          node:push(self:pop())
          commit = true
        end
      end
      if not commit then
        self:restore(backup)
        break
      end
    end
    self:push(node)
    return true
  end

  self:restore(backup)
end

function class:concatenation()
  local backup = self:backup()

  if self:repetition() then
    local node = abnf_node("concatenation", self:pop())
    while true do
      local backup = self:backup()
      local commit
      if self:c_wsp() then
        while self:c_wsp() do end
        if self:repetition() then
          node:push(self:pop())
          commit = true
        end
      end
      if not commit then
        self:restore(backup)
        break
      end
    end
    self:push(node)
    return true
  end

  self:restore(backup)
end

function class:repetition()
  local backup = self:backup()

  local repeat_
  if self:repeat_() then
    repeat_ = self:pop()
  end

  if self:element() then
    local node = abnf_node("repetition", self:pop(), repeat_)
    self:push(node)
    return true
  end

  self:restore(backup)
end

function class:repeat_()
  if self:match "(%d*)%*(%d*)" then -- *Rule
    self:push(abnf_node("repeat", "*", self[1], self[2]))
    return true
  elseif self:match "%d+" then -- nRule
    self:push(abnf_node("repeat", "n", self[0]))
    return true
  end
end

function class:element()
  if self:rulename() or self:group() or self:option() or self:char_val() or self:num_val() or self:prose_val() then
    local node = abnf_node("element", self:pop())
    self:push(node)
    return true
  end
end

function class:group()
  local backup = self:backup()
  if self:match "%(" then
    while self:c_wsp() do end
    if self:alternation() then
      while self:c_wsp() do end
      if self:match "%)" then
        local node = abnf_node("group", self:pop())
        self:push(node)
        return true
      end
    end
  end
  self:restore(backup)
end

function class:option()
  local backup = self:backup()
  if self:match "%[" then
    while self:c_wsp() do end
    if self:alternation() then
      while self:c_wsp() do end
      if self:match "%]" then
        local node = abnf_node("option", self:pop())
        self:push(node)
        return true
      end
    end
  end
  self:restore(backup)
end

function class:char_val()
  if self:match "\"[\32\33\35-\115]*\"" then
    self:push(abnf_node("char_val", self[0]))
    return true
  end
end

function class:num_val()
  local backup = self:backup()
  if self:match "%%" and (self:bin_val() or self:dec_val() or self:hex_val()) then
    local node = abnf_node("num_val", self:pop())
    self:push(node)
    return true
  end
  self:restore(backup)
end

function class:bin_val()
  if self:match "b[01]+" then
    local node = abnf_node("bin_val", self[0])
    if self:match "%.[01]+" then
      node:push(self[0])
      while self:match "%.[01]+" do
        node:push(self[0])
      end
    elseif self:match "%-[01]+" then
      node:push(self[0])
    end
    self:push(node)
    return true
  end
end

function class:dec_val()
  if self:match "d%d+" then
    local node = abnf_node("dec_val", self[0])
    if self:match "%.%d+" then
      node:push(self[0])
      while self:match "%.%d+" do
        node:push(self[0])
      end
    elseif self:match "%-%d+" then
      node:push(self[0])
    end
    self:push(node)
    return true
  end
end

function class:hex_val()
  if self:match "x%x+" then
    local node = abnf_node("dec_val", self[0])
    if self:match "%.%x+" then
      node:push(self[0])
      while self:match "%.%x+" do
        node:push(self[0])
      end
    elseif self:match "%-%x+" then
      node:push(self[0])
    end
    self:push(node)
    return true
  end
end

function class:prose_val()
  if self:match "<[\32-\61\63-\126]*>" then
    return true
  end
end

function metatable:__index(key)
  if type(key) == "number" then
    return self.matches[key]
  else
    return class[key]
  end
end

local abnf_parser = setmetatable(class, {
  __call = function (_, buffer)
    return setmetatable(new(buffer), metatable)
  end;
})

local function process(number, line_range_i, line_range_j)
  local path = ("rfc%04d.txt"):format(number)

  local page_number = 1
  local line_number = 0

  local buffer = {}

  for line in io.lines(path) do
    line_number = line_number + 1
    if line == "\f" then
      page_number = page_number + 1
    end
    if line_range_i <= line_number and line_number <= line_range_j then
      buffer[#buffer + 1] = line
    end
  end

  local indent = buffer[1]:match "^ *"
  if #indent == 0 then
    indent = nil
  else
    indent = "^" .. indent
  end

  for i = 1, #buffer do
    if buffer[i] == "\f" then
      if buffer[i - 1] then
        buffer[i - 1] = ""
      end
      buffer[i] = ""
      if buffer[i + 1] then
        buffer[i + 1] = ""
      end
    end
    if indent then
      buffer[i] = buffer[i]:gsub(indent, "")
    end
  end

  local buffer = (table.concat(buffer, "\n") .. "\n")
    -- :gsub("\r\n", "\n")
    -- :gsub("\f\n[^\n]*\n", "\f\n")   -- header
    -- :gsub("[^\n]*\n\f\n", "\n\f\n") -- footer
    -- :gsub("\f\n", "\n")
  -- io.write(buffer)

  parser = abnf_parser(buffer)
  parser:rulelist()

  -- print(buffer:sub(47, 57))
end

process(5234, 549, 627)
-- process(5234, 720, 778)

-- process(3986, 2697, 2788)

-- process(7230, 4555, 4683)

--[[
local p = abnf [[
abc = "a" / "b"
def = abc abc
p:rulename()

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
]]

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


--[[
local class = {}
local metatable = { __index = class }

local function new(source)
  return {
    source = source;
    p = 1;
  }
end

function class:rulename()
  local v = self.source:match "^([A-Za-z][A-Za-z0-9%-]*)"
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




return setmetatable(class, {
  __call = function (_, source)
    return setmetatable(new(source), metatable)
  end;
})

]]
