#! /usr/bin/env lua

local class = {}
local metatable = { __index = class }

local function new(name, ...)
  return { [0] = name, ... }
end

function class:push(node)
  self[#self + 1] = node
  return self
end

function class:dump_xml()
  io.write("<", self[0])
  if self.position then
    io.write(" position=\"", self.position, "\"")
  end
  io.write ">\n"
  for i = 1, #self do
    local that = self[i]
    if type(that) == "table" then
      that:dump_xml()
    else
      io.write("<value>", (that:gsub("&", "&amp;"):gsub("<", "&lt;")), "</value>\n")
    end
  end
  io.write("</", self[0], ">\n")
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

function class:node(name, ...)
  local node = abnf_node(name, ...)
  node.position = self.position
  return node
end

function class:top()
  local stack = self.stack
  return stack[#stack]
end

function class:push(node)
  local stack = self.stack
  stack[#stack + 1] = node
  return self
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

function class:rulelist()
  local node = self:node("rulelist")
  while true do
    local backup = self:backup()
    local commit
    if self:rule() then
      node:push(self:pop())
      commit = true
    else
      while self:c_wsp() do end
      if self:c_nl() then
        commit = true
      end
    end
    if not commit then
      self:restore(backup)
      break
    end
  end
  assert(#self.buffer + 1 == self.position)
  assert(#self.stack == 0)
  return node
end

function class:rule()
  local backup = self:backup()
  local node = self:node "rule"
  if self:rulename() then
    node:push(self:pop())
    if self:defined_as() then
      node:push(self:pop())
      if self:elements() then
        node:push(self:pop())
        if self:c_nl() then
          return self:push(node)
        end
      end
    end
  end
  self:restore(backup)
end

function class:rulename()
  local node = self:node "rulename"
  if self:match "%a[%a%d%-]*" then
    return self:push(node:push(self[0]))
  end
end

function class:defined_as()
  local backup = self:backup()
  local node = self:node "defined_as"
  while self:c_wsp() do end
  if self:match "=/?" then
    node:push(self[0])
    while self:c_wsp() do end
    return self:push(node)
  end
  self:restore(backup)
end

function class:elements()
  local node = self:node "elements"
  if self:alternation() then
    while self:c_wsp() do end
    return self:push(node:push(self:pop()))
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
  return self:comment() or self:match "\r\n"
end

function class:comment()
  return self:match ";[ \t\33-\126]*\r\n"
end

function class:alternation()
  local backup = self:backup()
  local node = self:node "alternation"
  if self:concatenation() then
    node:push(self:pop())
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
    return self:push(node)
  end
  self:restore(backup)
end

function class:concatenation()
  local backup = self:backup()
  local node = self:node "concatenation"
  if self:repetition() then
    node:push(self:pop())
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
    return self:push(node)
  end
  self:restore(backup)
end

function class:repetition()
  local backup = self:backup()
  local node = self:node "repetition"
  local repeat_node
  if self:repeat_() then
    repeat_node = self:pop()
  end
  if self:element() then
    return self:push(node:push(self:pop()):push(repeat_node))
  end
  self:restore(backup)
end

function class:repeat_()
  local node = self:node "repeat"
  if self:match "(%d*)%*(%d*)" then
    return self:push(node:push(self[1]):push(self[2]))
  elseif self:match "%d+" then
    return self:push(node:push(self[0]))
  end
end

function class:element()
  local node = self:node "element"
  if self:rulename() or self:group() or self:option() or self:char_val() or self:num_val() or self:prose_val() then
    return self:push(node:push(self:pop()))
  end
end

function class:group()
  local backup = self:backup()
  local node = self:node "group"
  if self:match "%(" then
    while self:c_wsp() do end
    if self:alternation() then
      while self:c_wsp() do end
      if self:match "%)" then
        return self:push(node:push(self:pop()))
      end
    end
  end
  self:restore(backup)
end

function class:option()
  local backup = self:backup()
  local node = self:node "option"
  if self:match "%[" then
    while self:c_wsp() do end
    if self:alternation() then
      while self:c_wsp() do end
      if self:match "%]" then
        return self:push(node:push(self:pop()))
      end
    end
  end
  self:restore(backup)
end

function class:char_val()
  local node = self:node "char_val"
  if self:match "\"[\32\33\35-\126]*\"" then
    return self:push(node:push(self[0]))
  end
end

function class:num_val()
  local backup = self:backup()
  local node = self:node "num_val"
  if self:match "%%" and (self:bin_val() or self:dec_val() or self:hex_val()) then
    node:push(self:pop())
    return self:push(node)
  end
  self:restore(backup)
end

function class:bin_val()
  local node = self:node "bin_val"
  if self:match "b([01]+)" then
    node:push(self[1])
    if self:match "%.([01]+)" then
      node:push "." :push(self[1])
      while self:match "%.([01]+)" do
        node:push(self[1])
      end
    elseif self:match "%-([01]+)" then
      node:push "-" :push(self[1])
    end
    return self:push(node)
  end
end

function class:dec_val()
  local node = self:node "dec_val"
  if self:match "d(%d+)" then
    node:push(self[1])
    if self:match "%.(%d+)" then
      node:push "." :push(self[1])
      while self:match "%.(%d+)" do
        node:push(self[1])
      end
    elseif self:match "%-(%d+)" then
      node:push "-" :push(self[1])
    end
    return self:push(node)
  end
end

function class:hex_val()
  local node = self:node "hex_val"
  if self:match "x(%x+)" then
    node:push(self[1])
    if self:match "%.(%x+)" then
      node:push "." :push(self[1])
      while self:match "%.(%x+)" do
        node:push(self[1])
      end
    elseif self:match "%-(%x+)" then
      node:push "-" :push(self[1])
    end
    return self:push(node)
  end
end

function class:prose_val()
  local node = self:node "prose_val"
  if self:match "<([\32-\61\63-\126]*)>" then
    return self:push(node:push(self[1]))
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
    line = line:gsub("\r$", "")
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

  local buffer = (table.concat(buffer, "\r\n") .. "\r\n")

  parser = abnf_parser(buffer)
  return parser:rulelist()
end

-- local list = process(5234, 549, 627)
local list = process(5234, 720, 778)
-- local list = process(3986, 2697, 2788)
-- local list = process(7230, 4555, 4683)

list:dump_xml()
