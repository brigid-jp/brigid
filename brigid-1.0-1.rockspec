package = "brigid"
version = "1.0-1"
source = {
  url = "https://github.com/brigid-jp/brigid/archive/v1.0.tar.gz";
  file = "brigid-1.0.tar.gz";
}
description = {
  summary = "brigid";
  license = "MIT";
  homepage = "https://github.com/brigid-jp/brigid/";
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
        brigid = {
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
