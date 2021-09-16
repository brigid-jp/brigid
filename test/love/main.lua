-- Copyright (c) 2019-2021 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local ffi = require "ffi"
local love = love
local brigid

local text
local text_height = 0
local recv_channel
local send_channel
local intr_channel

function love.load()
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
    local i = text:addf(message .. "\n", width - 100, "left", 0, text_height)
    text_height = text_height + text:getHeight(i)
  end
end

function love.draw()
  local width, height = love.window.getMode()
  local y = 50
  if height < text_height then
    y = height - text_height - 50
  end
  love.graphics.draw(text, 50, y)
end
