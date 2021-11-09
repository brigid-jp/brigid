## ビルドスクリプトについて

### LuaRocksの変数でよく使うもの

#### ディレクトリ

```
LUA_BINDIR
LUA_INCDIR
LUA_LIBDIR
```

#### フラグ

`LIBFLAG`はWindowsでだけ使う。UNIX系はlibtoolで共有リンクライブラリをビルドする。

```
CFLAGS
LIBFLAG
```

