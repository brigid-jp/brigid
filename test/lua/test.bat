@echo off

REM Copyright (c) 2019 <dev@brigid.jp>
REM This software is released under the MIT License.
REM https://opensource.org/licenses/mit-license.php

dir /b test_*.lua | lua5.1 test.lua windows
