#! /bin/sh -e

# Copyright (c) 2021 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

LUA_CPATH="../../src/lua/.libs/?.so;;"
export LUA_CPATH

case X$# in
  X0) lua bench.lua;;
  *) "$@" bench.lua;;
esac
