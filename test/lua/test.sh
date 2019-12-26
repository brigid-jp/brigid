#! /bin/sh -e

# Copyright (c) 2019 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

case X$# in
  X0) ls test_*.lua | lua test.lua autotools;;
  *) ls test_*.lua | "$@" test.lua autotools;;
esac
