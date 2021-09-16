-- Copyright (c) 2019-2021 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local ffi = require "ffi"
local love = love
local brigid
local hasher

local text_source = {}
local text

local recv_channel
local send_channel
local intr_channel

function love.load()
  -- cache java classes for android
  local result, message = pcall(function ()
    brigid = require "brigid"
    hasher = assert(brigid.hasher "sha256")
    hasher:update ""
    local result = hasher:digest()
    assert(result == table.concat {
      "\227\176\196\066\152\252\028\020";
      "\154\251\244\200\153\111\185\036";
      "\039\174\065\228\100\155\147\076";
      "\164\149\153\027\120\082\184\085";
    })
  end)
  if result then
    text_source[#text_source + 1] = "[PASS] main thread require: " .. tostring(brigid)
  else
    text_source[#text_source + 1] = "[FAIL] main thread require: " .. message
  end

  text = love.graphics.newText(love.graphics.getFont())
  recv_channel = love.thread.newChannel()
  send_channel = love.thread.newChannel()
  intr_channel = love.thread.newChannel()

  local thread = love.thread.newThread("thread.lua")
  thread:start(recv_channel, send_channel, intr_channel)
end

function love.update(dt)
  local width, height = love.window.getMode()
  while true do
    local message = recv_channel:pop()
    if not message then
      break
    end
    text_source[#text_source + 1] = message
  end
end

function love.draw()
  local width, height = love.window.getMode()

  local text_height = 0
  text:clear()
  for i = 1, #text_source do
    local t = text_source[i]
    -- for j = 1, #t, 40 do
    --   text:addf(t:sub(j, j + 39) .. "\n", width - 100, "left", 0, text_height)
    --   text_height = text_height + text:getHeight(i)
    -- end
    text:addf(t .. "\n", width - 100, "left", 0, text_height)
    text_height = text_height + text:getHeight(i)
  end

  local y = 50
  if height < text_height then
    y = height - text_height - 50
  end
  love.graphics.draw(text, 50, y)
end
