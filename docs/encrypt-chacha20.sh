#! /bin/sh -e

# Copyright (c) 2019 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

plain="The quick brown fox jumps over the lazy dog"
# 256bit: "01234567890123456789012345678901"
key=3031323334353637383930313233343536373839303132333435363738393031
# 128bit: "0123456778012345"
iv=30313233343536373839303132333435

printf "$plain" | openssl enc -e -chacha -out chacha20.dat -K "$key" -iv "$iv"
