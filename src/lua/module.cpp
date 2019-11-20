// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "common.hpp"

namespace brigid {
  void initialize_crypto(lua_State*);

  void initialize(lua_State* L) {
    initialize_crypto(L);
  }
}

extern "C" int luaopen_brigid(lua_State* L) {
  lua_newtable(L);
  brigid::initialize(L);
  return 1;
}
