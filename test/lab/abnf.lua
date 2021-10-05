#! /usr/bin/env lua

-- Copyright (c) 2021 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local config = {
  name = "abnf";
  prefix = "";
  suffix = "";

  dump_xml = true;
  dump_dot = true;

  { "rfc5234",  720,  778 };
  { "rfc3986", 2697, 2788 };
  -- { "rfc7230", 4555, 4683 };
  { "rfc7230", 4555, 4611 };
  -- the field-content rule is broken
  { "rfc7230", 4614, 4683 };
  { "rfc6455",  741,  747 };
  { "rfc6455", 1384, 1415 };
  { "rfc6455", 1421, 1423 };
  { "rfc6455", 1687, 1783 };
  { "errata" };
}

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
  -- io.write(("[DEBUIG] match %d %q\n"):format(self.position, self.source:sub(self.position, self.position + 16)), debug.traceback(), "\n")
  if i then
    -- io.write(("[DEBUIG] matched %d %d %q\n"):format(i, j, self.source:sub(i, j)), debug.traceback(), "\n")
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
  if self:match "=/" then
    error("incremental alternation not supported at position " .. self.position)
  elseif self:match "=" then
    node:push "="
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
      if self:match "[/|]" then
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
  if self["repeat"](self) then
    repeat_node = self:pop()
  end
  if self:element() then
    return self:push(node:push(self:pop()):push(repeat_node))
  end
  self:restore(backup)
end

class["repeat"] = function (self)
  local node = self:node "repeat"
  if self:match "(%d*)%*(%d*)" then
    return self:push(node:push(self[1]):push(self[2]))
  elseif self:match "(%d*)#(%d*)" then
    if not ((self[1] == "" or self[1] == "0" or self[1] == "1") and self[2] == "") then
      error(self[1] .. "#" .. self[2] .. "rule not supported")
    end
    node[0] = "list"
    return self:push(node:push(self[1]):push(self[2]):push(self:node("rulename", "OWS")))
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
  if self:match "\"([\32\33\35-\126]*)\"" then
    return self:push(node:push(self[1]))
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

local class = {}
local metatable = { __index = class }

local function new(node, endl, prefix, suffix)
  return {
    node = node;
    endl = endl;
    prefix = prefix;
    suffix = suffix;
  }
end

function class:push(...)
  local node = self.node
  local node_buffer = node[-2]
  local that_buffer = { ... }
  if not node_buffer then
    node_buffer = {}
    node[-2] = node_buffer
  end
  for i = 1, #that_buffer do
    node_buffer[#node_buffer + 1] = that_buffer[i]
  end
  return self
end

function class:copy(that)
  local node = self.node
  local node_buffer = node[-2]
  local that_buffer = that[-2]
  if not node_buffer then
    node_buffer = {}
    node[-2] = node_buffer
  end
  for i = 1, #that_buffer do
    node_buffer[#node_buffer + 1] = that_buffer[i]
  end
  return self
end

function class:rule(node)
  self:copy(node[1]):copy(node[2]):copy(node[3]):push ";"
end

function class:rulename(node)
  self:push(self.prefix .. node[1]:gsub("%-", "_") .. self.suffix)
end

function class:defined_as(node)
  assert(node[1] == "=")
  self:push " = "
end

function class:elements(node)
  self:copy(node[1])
end

function class:alternation(node)
  self:copy(node[1])
  for i = 2, #node do
    if node[i - 1].line < node[i].line then
      self:push(self.endl, "  | ")
    else
      self:push " | "
    end
    self:copy(node[i])
  end
end

function class:concatenation(node)
  self:copy(node[1])
  for i = 2, #node do
    if node[i - 1].line < node[i].line then
      self:push(self.endl, "  ")
    else
      self:push " "
    end
    self:copy(node[i])
  end
end

function class:repetition(node)
  if node[2] then
    if node[2][0] == "repeat" then
      self:copy(node[1]):copy(node[2])
    else
      assert(node[2][0] == "list")
      local n = tonumber(node[2][1]) or 0
      assert(n == 0 or n == 1)
      assert(node[2][2] == "")
      if n == 0 then
        -- https://github.com/brigid-jp/brigid/blob/develop/test/lab/rfc7230.txt#L3331
        -- #element => [ ( "," / element ) *( OWS "," [ OWS element ] ) ]
        self:push [[(("," | ]] :copy(node[1]):push [[) (OWS "," (OWS ]] :copy(node[1]):push [[)?)*)?]]
      else
        -- https://github.com/brigid-jp/brigid/blob/develop/test/lab/rfc7230.txt#L3333
        -- 1#element => *( "," OWS ) element *( OWS "," [ OWS element ] )
        self:push [[("," OWS)* ]] :copy(node[1]):push [[ (OWS "," (OWS ]]:copy(node[1]):push [[)?)*]]
      end
    end
  else
    self:copy(node[1])
  end
end

class["repeat"] = function (self, node)
  local a = node[1]
  local b = node[2]
  if b then
    local a = tonumber(a)
    local b = tonumber(b)
    if a then
      if b then
        if a == b then
          self:push("{", a, "}")
        else
          self:push("{", a, ",", b, "}")
        end
      else
        if a == 0 then
          self:push "*"
        elseif a == 1 then
          self:push "+"
        else
          self:push("{", a, ",}")
        end
      end
    else
      if b then
        self:push("{,", b, "}")
      else
        self:push "*"
      end
    end
  else
    self:push("{", a, "}")
  end
end

function class:list(node)
end

function class:element(node)
  self:copy(node[1])
end

function class:group(node)
  self:push "(" :copy(node[1]):push ")"
end

function class:option(node)
  self:push "(" :copy(node[1]):push ")?"
end

function class:char_val(node)
  self:push([["]], node[1]:gsub([[\]], [[\\]]), [["]])
  if node[1]:find "%a" then
    self:push "i"
  end
end

function class:num_val(node)
  self:copy(node[1])
end

function class:bin_val(node)
  self:push(("0x%X"):format(tonumber(node[1], 2)))
  local op = node[2]
  if op == "." then
    for i = 3, #node do
      self:push((" 0x%X"):format(tonumber(node[i], 2)))
    end
  elseif op == "-" then
    self:push(("..0x%X"):format(tonumber(node[3], 2)))
  end
end

function class:dec_val(node)
  self:push(node[1])
  local op = node[2]
  if op == "." then
    for i = 3, #node do
      self:push(" ", node[i])
    end
  elseif op == "-" then
    self:push("..", node[3])
  end
end

function class:hex_val(node)
  self:push("0x", node[1])
  local op = node[2]
  if op == "." then
    for i = 3, #node do
      self:push(" 0x", node[i])
    end
  elseif op == "-" then
    self:push("..0x", node[3])
  end
end

function class:prose_val(node)
  self:push("<", node[1], ">")
end

function metatable:__call()
  local node = self.node
  local name = node[0]
  local f = self[name]
  if not f then
    error(name .. " not supported")
  end
  f(self, node)
end

local abnf_generator = setmetatable(class, {
  __call = function (_, node, endl, prefix, suffix)
    return setmetatable(new(node, endl, prefix, suffix), metatable)
  end;
})

local function process(node, endl, prefix, suffix)
  for i = 1, #node do
    local that = node[i]
    if getmetatable(that) == getmetatable(node) then
      process(that, endl, prefix, suffix)
    end
  end
  abnf_generator(node, endl, prefix, suffix)()
end

local function generate(rule, endl, prefix, suffix)
  process(rule, endl, prefix, suffix)
  return table.concat(rule[-2])
end

local root = abnf_node "root"

local function process(basename, line_range_i, line_range_j)
  if not line_range_i then
    line_range_i = 1
  end
  local line_number = 0
  local buffer = {}
  for line in io.lines(basename .. ".txt") do
    line_number = line_number + 1
    line = line:gsub("\r$", ""):gsub("^[ \t]+$", "")
    if line_range_i <= line_number and (not line_range_j or line_number <= line_range_j) then
      buffer[#buffer + 1] = line
    end
  end
  if not line_range_j then
    line_range_j = line_number
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
    rule.last_line = last_line

    local rule_buffer = {}
    for i = rule.line, last_line do
      rule_buffer[#rule_buffer + 1] = buffer[i + 1 - line_range_i]
    end
    rule[-1] = rule_buffer

    if #(rule:find_by_name "prose_val") > 0 then
      rule.prose_val = true
    end

    if basename == "errata" then
      rule.erratum = true
    end

    rule.basename = basename
  end

  root:push(rulelist)
end

for i = 1, #config do
  process(table.unpack(config[i]))
end

local name_map = {}

for i = 1, #root do
  local rulelist = root[i]
  for j = 1, #rulelist do
    local rule = rulelist[j]
    local def_name = rule[1][1]

    local that = name_map[def_name]
    if that then
      io.write(([[
[%7s.txt:%4d] rule %q redefined
[%7s.txt:%4d] previously defined here
]]):format(rule.basename, rule.line, def_name, that.basename, that.line))

      if rule.prose_val then
        if rule.erratum then
          io.write "[===== INFO =====] later rule is erratum, win later\n"
          that.ignored = true
          name_map[def_name] = rule
        else
          io.write "[===== INFO =====] later rule has prose-val, win earlier\n"
          rule.ignored = true
        end
      elseif that.prose_val then
        if that.erratum then
          io.write "[===== INFO =====] earlier rule erratum, win earlier\n"
          rule.ignored = true
        else
          io.write "[===== INFO =====] earlier rule has prose-val, win later\n"
          that.ignored = true
          name_map[def_name] = rule
        end
      else
        local new_name = ("%s-%s"):format(rule.basename, def_name)
        assert(not name_map[new_name])
        io.write(("[===== WARN =====] neither rule has prose-val, rename %q to %q\n"):format(def_name, new_name))
        local function process(node)
          if node[0] == "rulename" and node[1] == def_name then
            io.write(("[%7s.txt:%4d] rename %q to %q\n"):format(rule.basename, node.line, def_name, new_name))
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
        error(("[%7s.txt:%4d] rule %q uses undefined rule %q"):format(rule.basename, rule.line, def_name, use_name))
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

  local loop = function () end

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
[%7s.txt:%4d] loop detected: rule %q uses rule %q
[%7s.txt:%4d] rule %q defined here
[===== WARN =====] modify rulename to prose-val

]]):format(node.basename, node.line, node[1][1], use_name, that.basename, that.line, use_name))

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
        node.loop = true
        node.prose_val = true

        error(loop)
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
    if message == loop then
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
for i = #id_map, 1, -1 do
  process(i)
end

if config.dump_xml then
  local out = assert(io.open(config.name .. ".xml", "w"))
  root:dump_xml(out)
  out:close()
end

if config.dump_dot then
  local out = assert(io.open(config.name .. ".dot", "w"))
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
end

local out = assert(io.open(config.name .. ".rl", "w"))
out:write [[
%%{
# vim: syntax=ragel:
machine abnf;
]]
for i = #order, 1, -1 do
  local rule = id_map[order[i]]
  out:write(([[

# https://github.com/brigid-jp/brigid/blob/develop/test/lab/%s.txt#L%d
]]):format(rule.basename, rule.line))
  local buffer = rule[-1]
  for k = 1, #buffer do
    out:write("# ", buffer[k], "\n")
  end
  if rule.prose_val then
    out:write("# ", generate(rule, "\n# ", config.prefix, config.suffix), "\n")
  else
    out:write(generate(rule, "\n", config.prefix, config.suffix), "\n")
  end
end
out:write [[
}%%
]]
out:close()
