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
  type = "command";
  build_command = [[env PATH="$(LUA_BINDIR):$PATH" CPPFLAGS="$CPPFLAGS -I$(LUA_INCDIR)" CXXFLAGS="$CXXFLAGS -Wall -W -Wno-missing-field-initializers $(CFLAGS)" LUA="$(LUA)" ./configure --prefix="$(PREFIX)" && make]];
  install_command = [[make luadir="$(LUADIR)" luaexecdir="$(LIBDIR)" install]];
  platforms = {
    windows = {
      type = "builtin";
      modules = {
        brigid_core = {
          sources = {
            "common.cpp";
            "module.cpp";
            "crypto_windows.cpp";
          };
        };
      };
    };
  };
}
