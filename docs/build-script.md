## ビルドスクリプトについて

### LuaRocksの変数でよく使うもの

#### ディレクトリ

```
LUA_BINDIR
LUA_INCDIR
LUA_LIBDIR
```

#### フラグ

`LIBFLAG`はWindowsでだけ使う。
UNIX系はlibtoolで共有リンクライブラリをビルドする。
`LUALIB`はWindowsだけでなく、Java用のドライバ作成でも使える。

```
CFLAGS
LIBFLAG
LUALIB
```

### Makeの既定変数

```
CC
```

### Makeの設定変数

```
CPPFLAGS
CXXFLAGS
```

