#! /usr/bin/env lua

-- Copyright (c) 2021 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local class = {}
local metatable = { __index = class }

local function new(name, ...)
  return { [0] = name, ... }
end

local escape_map = {
  ["&"] = "&amp;";
  ["<"] = "&lt;";
  [">"] = "&gt;";
  ["\""] = "&quot;";
  ["'"] = "&apos;";
}

local function escape(source)
  return (source:gsub("[&<>\"']", escape_map))
end

function class:push(node)
  self[#self + 1] = node
  return self
end

function class:find_by_name(name, result)
  if not result then
    result = {}
  end

  if self[0] == name then
    result[#result + 1] = self
  end

  for i = 1, #self do
    local that = self[i]
    if getmetatable(that) == metatable then
      result = that:find_by_name(name, result)
    end
  end

  return result
end

function class:dump_xml(out)
  if not out then
    out = io.stdout
  end

  out:write("<", self[0])
  local keys = {}
  for k, v in pairs(self) do
    if type(k) == "string" then
      keys[#keys + 1] = k
    end
  end
  table.sort(keys)
  for i = 1, #keys do
    local k = keys[i]
    out:write(" ", escape(k), "=\"", escape(tostring(self[k])), "\"")
  end
  out:write ">\n"

  for i = 1, #self do
    local that = self[i]
    if getmetatable(that) == metatable then
      that:dump_xml(out)
    else
      out:write("<value>", escape(tostring(that)), "</value>\n")
    end
  end
  out:write("</", self[0], ">\n")
end

local abnf_node = setmetatable(class, {
  __call = function (_, name, ...)
    return setmetatable(new(name, ...), metatable)
  end;
})

local class = {}
local metatable = {}

local function new(source)
  return {
    source = source;
    position = 1;
    stack = {};
    matches = {};
  }
end

local function match(self, i, j, ...)
  if i then
    self.position = j + 1
    self.matches = { [0] = self.source:sub(i, j), ... }
    return true
  else
    return false
  end
end

function class:match(pattern)
  return match(self, self.source:find("^" .. pattern, self.position))
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
  assert(#self.source + 1 == self.position)
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
  __call = function (_, source)
    return setmetatable(new(source), metatable)
  end;
})

local root = abnf_node "root"

local function process(number, line_range_i, line_range_j)
  local path = ("rfc%04d.txt"):format(number)

  local line_number = 0

  local buffer = {}

  for line in io.lines(path) do
    line_number = line_number + 1
    line = line:gsub("\r$", ""):gsub("^[ \t]+$", "")
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

  local source = (table.concat(buffer, "\r\n") .. "\r\n")

  local source_map = {}
  local position = 0
  for i = 1, #buffer do
    local line_number = i - 1 + line_range_i
    local n = #buffer[i] + 2
    for j = position + 1, position + n do
      source_map[j] = line_number
    end
    position = position + n
  end

  parser = abnf_parser(source)
  local rulelist = parser:rulelist()

  local function process(node)
    if node.position then
      node.line = source_map[node.position]
    end
    for i = 1, #node do
      local that = node[i]
      if getmetatable(that) == getmetatable(node) then
        process(that)
      end
    end
  end
  process(rulelist)

  for i = 1, #rulelist do
    local rule = rulelist[i]
    local that = rulelist[i + 1]

    local last_line
    if that then
      last_line = that.line - 1
    else
      last_line = line_range_j
    end
    while buffer[last_line + 1 - line_range_i] == "" do
      last_line = last_line - 1
    end

    local rule_buffer = {}
    for i = rule.line, last_line do
      rule_buffer[#rule_buffer + 1] = buffer[i + 1 - line_range_i]
    end

    local prose_val
    if #(rule:find_by_name "prose_val") > 0 then
      prose_val = true
    end

    rule[-1] = rule_buffer
    rule.last_line = last_line
    rule.prose_val = prose_val
    rule.rfc_number = number
  end

  root:push(rulelist)
end

process(5234, 720, 778)
-- process(5234, 549, 627)
process(3986, 2697, 2788)
process(7230, 4555, 4683)

local name_map = {}

for i = 1, #root do
  local rulelist = root[i]
  for j = 1, #rulelist do
    local rule = rulelist[j]
    local def_name = rule[1][1]

    local that = name_map[def_name]
    if that then
      io.write(([[
[rfc%d.txt:%4d] redefined rule %q
[rfc%d.txt:%4d] previously defined here
]]):format(rule.rfc_number, rule.line, def_name, that.rfc_number, that.line))

      if rule.prose_val then
        io.write "[INFO] later rule has prose-val, win first\n"
        rule.ignored = true
      elseif that.prose_val then
        io.write "[INFO] first rule has prose-val, win later\n"
        that.ignored = true
        name_map[def_name] = rule
      else
        local new_name = ("rfc%d-%s"):format(rule.rfc_number, def_name)
        assert(not name_map[new_name])
        io.write(("[WARN] neither rule has prose-val, rename %q to %q\n"):format(def_name, new_name))
        local function process(node)
          if node[0] == "rulename" and node[1] == def_name then
            io.write(("[rfc%d.txt:%4d] rename %q to %q\n"):format(rule.rfc_number, node.line, def_name, new_name))
            node[1] = new_name
          end
          for i = 1, #node do
            local that = node[i]
            if getmetatable(that) == getmetatable(node) then
              process(that)
            end
          end
        end
        process(rulelist)
        name_map[new_name] = rule
      end
      io.write "\n"
    else
      name_map[def_name] = rule
    end
  end
end

local id_map = {}

local id = 0
for i = 1, #root do
  local rulelist = root[i]
  for j = 1, #rulelist do
    local rule = rulelist[j]
    if not rule.ignored then
      id = id + 1
      rule.id = id
      id_map[id] = rule
    end
  end
end

local use_map

repeat
  use_map = {}

  for i = 1, #id_map do
    local rule = id_map[i]
    local def_name = rule[1][1]

    local rulename = rule[3]:find_by_name "rulename"
    local use_id_map = {}
    for j = 1, #rulename do
      local use_name = rulename[j][1]
      local use_rule = name_map[use_name]
      if not use_rule then
        error(("[rfc%d.txt:%4d] rule %q uses undefined rule %q"):format(rule.rfc_number, rule.line, def_name, use_name))
      end
      use_id_map[use_rule.id] = true
    end

    local use_ids = {}
    for k in pairs(use_id_map) do
      use_ids[#use_ids + 1] = k
    end
    table.sort(use_ids)

    use_map[i] = use_ids
  end

  local loop_detector = function () end

  local color = {}
  local function process(id)
    color[id] = 1
    local use_ids = use_map[id]
    for i = 1, #use_ids do
      local use_id = use_ids[i]
      local c = color[use_id]
      if not c then
        process(use_id)
      elseif c == 1 then
        local node = id_map[id]
        local that = id_map[use_id]
        local use_name = that[1][1]
        io.write(([[
[rfc%d.txt:%4d] loop detected at rule %q uses rule %q
[rfc%d.txt:%4d] rule %q is defined here
[WARN] modify rulename to prose_val

]]):format(node.rfc_number, node.line, node[1][1], use_name, that.rfc_number, that.line, use_name))

        local function process(node)
          if node[0] == "rulename" and node[1] == use_name then
            node[0] = "prose_val"
          end
          for i = 1, #node do
            local that = node[i]
            if getmetatable(that) == getmetatable(node) then
              process(that)
            end
          end
        end
        process(node)
        node.prose_val = true

        error(loop_detector)
      end
    end
    color[id] = 2
  end

  local result, message = pcall(function ()
    for i = 1, #id_map do
      process(i)
    end
  end)

  local loop_detected
  if not result then
    if message == loop_detector then
      loop_detected = true
    else
      error(message)
    end
  end
until not loop_detected

local ref_map = {}

for i = 1, #id_map do
  ref_map[i] = {}
end
for i = 1, #use_map do
  local use_ids = use_map[i]
  for j = 1, #use_ids do
    local use_id = use_ids[j]
    local ref_ids = ref_map[use_id]
    ref_ids[#ref_ids + 1] = i
  end
end

-- topological sort
local order = {} -- reversed
local color = {}

local function process(id)
  if not color[id] then
    color[id] = true
    local ref_ids = ref_map[id]
    for i = 1, #ref_ids do
      process(ref_ids[i])
    end
    order[#order + 1] = id
  end
end
for i = 1, #id_map do
  process(i)
end

root:dump_xml(assert(io.open("abnf.xml", "w")))

local out = assert(io.open("abnf.dot", "w"))
out:write [[
digraph {
graph[rankdir=LR];
]]
for i = 1, #id_map do
  local rule = id_map[i]
  out:write(([[
%d [label="%s"];
]]):format(i, rule[1][1]))
end
for i = 1, #ref_map do
  local ids = ref_map[i]
  for j = 1, #ids do
    out:write(([[
%d -> %d;
]]):format(i, ids[j]))
  end
end
out:write "}\n"
out:close()

--[====[

for i = 1, #root do
  local rulelist = root[i]
  for j = 1, #rulelist do
    local rule = rulelist[j]
    io.write(([[
# https://github.com/brigid-jp/brigid/blob/develop/test/lab/rfc%d.txt#L%d
]]):format(rule.rfc_number, rule.line))
    local buffer = rule[-1]
    for k = 1, #buffer do
      io.write("# ", buffer[k], "\n")
    end
    io.write "\n"
  end
end

]====]
