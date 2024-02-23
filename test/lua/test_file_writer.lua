-- Copyright (c) 2021 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

local brigid = require "brigid"
local test_suite = require "test_suite"

local suite = test_suite "test_file_writer"
local debug = false

function suite:test_file_writer1()
  local file_writer = assert(brigid.file_writer(test_cwd .. "/test.dat"))
  assert(file_writer:write "foo\n")
  assert(file_writer:write "bar\n")
  assert(file_writer:write "baz\n")
  assert(file_writer:write "qux\n")
  assert(file_writer:close())
  assert(file_writer:close())

  local result, message = pcall(function () file_writer:write "foobarbazqux" end)
  if debug then print(message) end
  assert(not result)
  assert(message:find "bad self" or message:find "bad argument")

  local handle = assert(io.open(test_cwd .. "/test.dat"))
  assert(handle:read "*a" == "foo\nbar\nbaz\nqux\n")
  handle:close()
  os.remove(test_cwd .. "test.dat")
end

function suite:test_file_writer2()
  local result, message = brigid.file_writer(test_cwd .. "/no such directory/test.dat")
  if debug then print(message) end
  assert(not result)
end

function suite:test_file_writer3()
  local path = test_cwd .. "/test.dat"
  local file_writer = assert(brigid.file_writer(path))

  for i = 1, 16 do
    assert(file_writer:write(("a"):rep(257)))

    local handle = assert(io.open(path, "rb"))
    local n = #handle:read "*a"
    handle:close()
    if debug then print(i * 257, n) end
    assert(i * 257 >= n)
  end

  assert(file_writer:flush())
  local handle = assert(io.open(path, "rb"))
  local n = #handle:read "*a"
  handle:close()
  if debug then print(16 * 257, n) end
  assert(16 * 257 == n)

  assert(file_writer:close())
  os.remove(path)
end

function suite:test_file_writer_write_urlencoded()
  local expect = "%E3%82%AD%E3%83%BC1=%E5%80%A41&%E3%82%AD%E3%83%BC2=%E5%80%A42&%E3%82%AD%E3%83%BC3=%E5%80%A43"

  local path = test_cwd .. "/test.dat"
  local file_writer = assert(brigid.file_writer(path))

  file_writer:write_urlencoded("キー1"):write"=":write_urlencoded("値1")
    :write"&":write_urlencoded("キー2"):write"=":write_urlencoded("値2")
    :write"&":write_urlencoded("キー3"):write"=":write_urlencoded("値3")
  assert(file_writer:close())

  local handle = assert(io.open(path))
  assert(handle:read "*a" == expect)
  handle:close()

  os.remove(path)
end

return suite
