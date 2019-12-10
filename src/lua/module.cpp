// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <lua.hpp>

namespace brigid {
  void initialize_crypto(lua_State*);
  void initialize_http(lua_State*);
  void initialize_version(lua_State*);
  void initialize_view(lua_State*);

  void initialize(lua_State* L) {
    initialize_crypto(L);
    initialize_http(L);
    initialize_version(L);
    initialize_view(L);
  }
}

extern "C" int luaopen_brigid(lua_State* L) {
  lua_newtable(L);
  brigid::initialize(L);
  return 1;
}
