R""--(
-- Copyright (c) 2024 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local brigid = ...

local function getmetafield(u, event)
  local metatable = getmetatable(u)
  if metatable then
    return metatable[event]
  end
end

local function write_json(self, u)
  local t = type(u)
  if t == "number" then
    return assert(self:write_json_number(u))
  elseif t == "string" then
    return assert(self:write_json_string(u))
  elseif t == "boolean" then
    if u then
      return assert(self:write "true")
    else
      return assert(self:write "false")
    end
  elseif t == "table" then
    if u[1] ~= nil or getmetafield(u, "__name") == "brigid.json.array" then
      assert(self:write "[")
      for i, v in ipairs(u) do
        if i > 1 then
          assert(self:write ",")
        end
        assert(write_json(self, v))
      end
      return assert(self:write "]")
    else
      local first = true
      assert(self:write "{")
      for k, v in pairs(u) do
        if first then
          first = false
        else
          assert(self:write ",")
        end
        assert(self:write_json_string(tostring(k)))
        assert(self:write ":")
        assert(write_json(self, v))
      end
      return assert(self:write "}")
    end
  else
    if u == brigid.null then
      return assert(self:write "null")
    else
      return assert(self:write_json_string(tostring(k)))
    end
  end
end

brigid.data_writer.write_json = write_json
brigid.file_writer.write_json = write_json

--)"--"
