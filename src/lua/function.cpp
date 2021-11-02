// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "common.hpp"
#include "function.hpp"

#include <lua.hpp>

namespace brigid {
  namespace detail {
    void set_field(lua_State* L, int index, const char* key, lua_CFunction value) {
      index = abs_index(L, index);
      lua_pushcfunction(L, value);
      lua_setfield(L, index, key);
    }

    void set_metafield(lua_State* L, int index, const char* key, lua_CFunction value) {
      if (lua_getmetatable(L, index)) {
        lua_pushcfunction(L, value);
        lua_setfield(L, -2, key);
        lua_pop(L, 1);
      } else {
        index = abs_index(L, index);
        lua_newtable(L);
        lua_pushcfunction(L, value);
        lua_setfield(L, -2, key);
        lua_setmetatable(L, index);
      }
    }
  }
}
