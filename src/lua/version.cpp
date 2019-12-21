// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/version.hpp>
#include "common.hpp"

#include <lua.hpp>

namespace brigid {
  namespace {
    void impl_get_version(lua_State* L) {
      push(L, get_version());
    }
  }

  void initialize_version(lua_State* L) {
    set_field(L, -1, "get_version", impl_get_version);
  }
}
