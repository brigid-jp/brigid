#! /bin/sh -e

# Copyright (c) 2019 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

case X$LUA_CPATH in
  X) LUA_CPATH="../../src/lua/.libs/?.so;;";;
  *) LUA_CPATH="../../src/lua/.libs/?.so;$LUA_CPATH";;
esac
export LUA_CPATH

for i in test*.lua
do
  case X$# in
    X0) lua "$i";;
    *) "$@" "$i";;
  esac
done
