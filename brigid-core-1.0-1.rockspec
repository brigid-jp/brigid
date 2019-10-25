package = "brigid-core"
version = "1.0-1"
source = {
  url = "https://github.com/brigid-jp/brigid-core/archive/v1.0.tar.gz";
  file = "brigid-core-1.0.tar.gz";
}
description = {
  summary = "brigid-core";
  license = "MIT";
  homepage = "https://github.com/brigid-jp/brigid-core/";
  maintainer = "dev@brigid.jp";
}
build = {
  type = "make";
  build_variables = {
    CFLAGS = "$(CFLAGS)";
    LIBFLAG = "$(LIBFLAG)";
    LUA_INCDIR = "$(LUA_INCDIR)";
    LUA_LIBDIR = "$(LUA_LIBDIR)";
  };
  install_variables = {
    LIBDIR = "$(LIBDIR)";
  };
}
