#! /bin/sh -e

# Copyright (c) 2019 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

plain="The quick brown fox jumps over the lazy dog"
password=password
export password

printf "$plain" | openssl enc -e -aes-256-cbc -out aes-256-cbc-salted.dat -pass env:password -md sha256
