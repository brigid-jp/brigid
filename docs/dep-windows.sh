#! /bin/sh -e

# Copyright (c) 2021 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

lua_version=5.1.5
dll_version=12
mod_version=3.7.0

here=`pwd`

dir=$1
arch=$2
case X$arch in
  X32) out=lua-x86;;
  X64) out=lua-x64;;
  *) exit 1;;
esac
dir=`(cd "$dir" && pwd)`

bin_filename="$dir/lua-${lua_version}_Win${arch}_bin.zip"
lib_filename="$dir/lua-${lua_version}_Win${arch}_dll${dll_version}_lib.zip"
mod_filename="$dir/luarocks-${mod_version}-windows-$arch.zip"

case X$TMPDIR in
  X) TMPDIR=/tmp;;
esac
tmp=`(umask 077 && mktemp -d "$TMPDIR/dromozoa-XXXXXX" 2>/dev/null || :)`
case X$tmp in
  X) tmp=$TMPDIR/dromozoa-$$-$RANDOM; (umask 077 && mkdir "$tmp");;
esac
cd "$tmp"
tmp=`pwd`
trap "(cd / && rm -fr '$tmp')" 0

cd "$tmp"
mkdir -p "$out/bin" "$out/include" "$out/lib" bin lib mod

unzip -q "$bin_filename" -d bin
unzip -q "$lib_filename" -d lib
unzip -q "$mod_filename" -d mod

cp bin/lua5.1.dll bin/lua5.1.exe bin/lua51.dll "mod/luarocks-${mod_version}-windows-$arch"/*.exe "$out/bin"
cp lib/include/*.h* "$out/include"
cp lib/lua51.lib "$out/lib/lua5.1.lib"

# bin
# bin/lua51.dll.manifest
# bin/bin2c5.1.exe
# bin/lua5.1.dll
# bin/lua5.1.exe.manifest
# bin/sample.wlua
# bin/lua5.1.exe
# bin/lua51.dll
# bin/lua5.1.dll.manifest
# bin/wlua5.1.exe
# bin/wlua5.1.exe.manifest
# bin/Microsoft.VC80.CRT
# bin/Microsoft.VC80.CRT/Microsoft.VC80.CRT.manifest
# bin/Microsoft.VC80.CRT/msvcm80.dll
# bin/Microsoft.VC80.CRT/msvcr80.dll
# bin/Microsoft.VC80.CRT/msvcp80.dll
# bin/luac5.1.exe
# lib
# lib/lua5.1.lib
# lib/lua5.1.dll
# lib/include
# lib/include/luaconf.h
# lib/include/lua.hpp
# lib/include/lualib.h
# lib/include/lauxlib.h
# lib/include/lua.h
# lib/lua51.lib
# lib/lua51.dll
# mod
# mod/luarocks-3.7.0-windows-32
# mod/luarocks-3.7.0-windows-32/luarocks-admin.exe
# mod/luarocks-3.7.0-windows-32/luarocks.exe

zip -r9 "$here/$out.zip" "$out"
