@echo off

REM Copyright (c) 2019 <dev@brigid.jp>
REM This software is released under the MIT License.
REM https://opensource.org/licenses/mit-license.php

set LUA=lua5.1
set LUA_CPATH=..\..\?.dll;;

for %%i in (test_*.lua) do (
  %LUA% %%i
  echo errorlevel=%ERRORLEVEL%
)
