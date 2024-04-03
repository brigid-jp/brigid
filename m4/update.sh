#! /bin/sh -e

# Copyright (c) 2019,2021,2024 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

for i in ax_check_openssl.m4 ax_compare_version.m4 ax_cxx_compile_stdcxx.m4 ax_lib_curl.m4 ax_lua.m4 ax_path_generic.m4
do
  curl -L "https://git.savannah.gnu.org/gitweb/?p=autoconf-archive.git;a=blob_plain;f=m4/$i" >"$i"
done

curl -LO https://git.savannah.gnu.org/cgit/gnulib.git/plain/m4/ld-version-script.m4
