-- Copyright (c) 2019-2021 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local ffi = require "ffi"
local love = love

local text_source = {}
local text_x = 0
local text_y = 0
local text_h = 0
local text_drag

local recv_channel
local send_channel
local intr_channel

function love.load()
  -- cache java classes for android
  local result, message = pcall(function ()
    require "brigid"
  end)

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
    local i = text:add(message, 0, text_h)
    text_h = text_h + text:getHeight(i)
  end
end

function love.draw()
  love.graphics.draw(text, text_x, text_y)
end

function love.mousemoved(x, y, dx, dy, is_touch)
  if text_drag then
    text_x = text_x + dx
    text_y = text_y + dy
  end
end

function love.mousepressed(x, y, button, is_touch, presses)
  if button == 1 then
    text_drag = true
  end
end

function love.mousereleased(x, y, button, is_touch, presses)
  if button == 1 then
    text_drag = nil
  end
end

function love.wheelmoved(x, y)
  print("W", x, y)
end
