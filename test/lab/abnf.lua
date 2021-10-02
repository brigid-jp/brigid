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
    group = {};
  }
end

local function match(self, i, j, ...)
  if i then
    self.position = j + 1
    self.group = { [0] = self.buffer:sub(i, j), ... }
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
  print("push", node[0], table.unpack(node))
end

function class:pop()
  local stack = self.stack
  local n = #stack
  local node = stack[n]
  stack[n] = nil
  print("pop", node[0], table.unpack(node))
  return node
end

function class:error()
  error("parser error: at position " .. self.position)
end

function rulelist(self)
  local position = self.position

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

  self.position = position
end

function class:rulelist()
  self:push(abnf_node("rulelist"))

  -- RFC5234の記述では最初のruleの前の空白が許容されない
  while self:c_wsp() do end

  if not rulelist(self) then
    self:error()
  end

  while true do
    if not rulelist() then
      break
    end
  end

  -- 全部読んだかをチェックする
  -- トップにrulelistがあるかをチェックする
end

function class:rule()
  local position = self.position

  if self:rulename() and self:defined_as() and self:elements() and self:c_nl() then
    -- rule nodeを作る
    return true
  end

  self.position = position
end

function class:rulename()
  if self:match "%a[%a%d%-]*" then
    self:push(abnf_node("rulename", self[0]))
    return true
  end
end

function class:defined_as()
  local position = self.position

  while self:c_wsp() do end
  if self:match "=/?" then
    self:push(abnf_node("defined_as", self[0]))
    while self:c_wsp() do end
    return true
  end

  self.position = position
end

function class:elements()
  if self:alternation() then
    while self:c_wsp() do end
    return true
  end
end

function class:c_wsp()
  local position = self.position

  if self:match "[ \t]" then
    return true
  else
    if self:c_nl() then
      if self:match "[ \t]" then
        return true
      end
    end
  end

  self.position = position
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
  local position = self.position

  if self:concatenation() then
    while true do
      local position = self.position

      while self:c_wsp() do end
      if self:match "/" then
        while self:c_wsp() do end
        if self:concatenation() then
          -- self:push("alternation", self[0])
          return true
        end

      end

      self.position = position
    end
    return true
  end

  self.position = position
end

function class:repetition()
  local node = abnf_node("repetition")
  if self:repeat_() then
    node:push(self:pop())
  end



end

function class:repeat_()
  if self:match "(%d*)%*(%d*)" then -- *Rule
    self:push("repeat", "*", self[1], self[2])
    return true
  elseif self:match "%d+" then -- nRule
    self:push("repeat", "n", self[0])
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
  local position = self.position
  if self:match "%(" then
    while self:c_wsp() do end
    if self:alternation() then
      while self:c_wsp() do end
      if self:match ")" then
        local node = abnf_node("group", self:pop())
        self:push(node)
        return true
      end
    end
  end
  self.position = position
end

function class:option()
  local position = self.position
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
  self.position = position
end

function class:char_val()
  if self:match "\"[\32\33\35-\115]*\"" then
    self:push(abnf_node("char_val", self[0]))
    return true
  end
end

function class:num_val()
  local position = self.position
  if self:match "%%" and (self:bin_val() or self:dec_val() or self:hex_val()) then
    local node = abnf_node("num_val", self:pop())
    self:push(node)
    return true
  end
  self.position = position
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
    return self.group[key]
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

  local line_number_to_page_number = {}
  local buffer = {}

  for line in io.lines(path) do
    line_number = line_number + 1
    if line == "\f" then
      page_number = page_number + 1
    end
    if line_range_i <= line_number and line_number <= line_range_j then
      buffer[#buffer + 1] = line
      line_number_to_page_number[line_number] = page_number
    end
  end

  local buffer = (table.concat(buffer, "\n") .. "\n")
    :gsub("\r\n", "\n")
    :gsub("\f\n[^\n]*\n", "\f\n")   -- header
    :gsub("[^\n]*\n\f\n", "\n\f\n") -- footer
    :gsub("\f\n", "\n")

  local match_position = 1
  local match_result
  local function match(pattern)
    local i, j, result = buffer:find("^" .. pattern, match_position)
    -- print(i, j, result)
    if i then
      match_position = j + 1
      if result then
        match_result = result
      else
        match_result = buffer:sub(i, j)
      end
      return true
    end
  end

  parser = abnf_parser(buffer)
  parser:rulelist()
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
