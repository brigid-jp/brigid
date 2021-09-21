#! /bin/sh -e

# Copyright (c) 2019 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

case X$# in
  X0) lua test_main.lua;;
  *) "$@" test_main.lua;;
esac
