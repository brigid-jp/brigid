#! /bin/sh -e

for i in ax_cxx_compile_stdcxx.m4 ax_lua.m4
do
  curl -L "https://git.savannah.gnu.org/gitweb/?p=autoconf-archive.git;a=blob_plain;f=m4/$i" >"$i"
done
