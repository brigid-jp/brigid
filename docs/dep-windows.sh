#! /bin/sh -e

# Copyright (c) 2021 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

lua_version=5.1.5
msvc_version=12
luarocks_version=3.7.0

dir=$1
arch=$2

bin_filename="$dir/lua-${lua_version}_Win${arch}_bin.zip"
lib_filename="$dir/lua-${lua_version}_Win${arch}_dll${msvc_version}_lib.zip"
luarocks_filename="$dir/luarocks-${luarocks_version}-windows-$arch.zip"

ls -l "$bin_filename" "$lib_filename" "$luarocks_filename"

unzip -l "$bin_filename"
unzip -l "$lib_filename"
unzip -l "$luarocks_filename"
