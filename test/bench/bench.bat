@echo off

REM Copyright (c) 2021 <dev@brigid.jp>
REM This software is released under the MIT License.
REM https://opensource.org/licenses/mit-license.php

SET LUA_CPATH=..\..\?.dll;;
lua5.1 bench.lua
