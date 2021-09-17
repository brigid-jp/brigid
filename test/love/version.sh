#! /bin/sh -e

# Copyright (c) 2021 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

version=`cat ../../brigid_version.m4`
expr "X$version" : 'X.*BRIGID_VERSION,\([0-9][0-9]*\.[0-9][0-9]*\)'
