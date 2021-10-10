#! /usr/bin/env lua

-- Copyright (c) 2021 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local range_x, range_y = ...
range_x = tonumber(range_x)
range_y = tonumber(range_y)

local function breakdown(abcd)
  local d = abcd & 0xF
  local abc = abcd >> 4
  local c = abc & 0xF
  local ab = abc >> 4
  local b = ab & 0xF
  local a = ab >> 4
  return abc, a, b, c, d
end

local function update(root, x, y)
  local range

  for abcd = x, y do
    local abc, a, b, c, d = breakdown(abcd)
    local next_abc = breakdown(abcd + 1)

    if range then
      range[2] = d
    else
      range = { d, d }
    end

    if abc ~= next_abc or abcd == y then
      local regex = ("[%X-%X]"):format(range[1], range[2])

      if not root[a] then
        root[a] = {}
      end
      if not root[a][b] then
        root[a][b] = {}
      end
      root[a][b][c] = {
        regex = regex;
      }

      range = nil
    end
  end
end

local function merge_regex(source, prepend)
  local result = {}
  for item in (source .. "\n"):gmatch "(.-)\n" do
    result[#result + 1] = prepend .. item
  end
  return table.concat(result, "\n")
end

local function merge(root)
  local regex_map = {}
  local range_map = {}
  for a = 0, 15 do
    local av = root[a]
    if av then
      do
        local regex_map = {}
        local range_map = {}
        for b = 0, 15 do
          local bv = av[b]
          if bv then
            do
              local regex_map = {}
              local range_map = {}
              for c = 0, 15 do
                local cv = bv[c]
                if cv then
                  if range_map[cv.regex] then
                    range_map[cv.regex][2] = c
                  else
                    regex_map[#regex_map + 1] = cv
                    range_map[cv.regex] = { c, c }
                  end
                end
              end
              local regex = {}
              for i = 1, #regex_map do
                local cv = regex_map[i]
                local range = range_map[cv.regex]
                regex[i] = ("[%X-%X]%s"):format(range[1], range[2], cv.regex)
              end
              if #regex == 1 then
                bv.regex = regex[1]
              else
                bv.regex = table.concat(regex, "\n")
              end
            end
            if range_map[bv.regex] then
              range_map[bv.regex][2] = b
            else
              regex_map[#regex_map + 1] = bv
              range_map[bv.regex] = { b, b }
            end
          end
        end
        local regex = {}
        for i = 1, #regex_map do
          local bv = regex_map[i]
          local range = range_map[bv.regex]
          regex[i] = merge_regex(bv.regex, ("[%X-%X]"):format(range[1], range[2]))
        end
        if #regex == 1 then
          av.regex = regex[1]
        else
          av.regex = table.concat(regex, "\n")
        end
      end
      if range_map[av.regex] then
        range_map[av.regex][2] = a
      else
        regex_map[#regex_map + 1] = av
        range_map[av.regex] = { a, a }
      end
    end
  end
  local regex = {}
  for i = 1, #regex_map do
    local av = regex_map[i]
    local range = range_map[av.regex]
    regex[i] = merge_regex(av.regex, ("[%X-%X]"):format(range[1], range[2]))
  end
  if #regex == 1 then
    root.regex = regex[1]
  else
    root.regex = table.concat(regex, "\n")
  end
end

local root = {}
update(root, range_x, range_y)
merge(root)
print(root.regex)
