// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "common.hpp"

#include <lua.hpp>

namespace brigid {
  namespace {
    void impl_array(lua_State* L) {
      lua_newtable(L);
      set_metatable(L, "brigid.json.array");
    }
  }

  void initialize_json_parse(lua_State*);

  void initialize_json(lua_State* L) {
    new_metatable(L, "brigid.json.array");
    lua_getglobal(L, "table");
    lua_setfield(L, -2, "__index");
    lua_pushboolean(L, true);
    lua_setfield(L, -2, "brigid.json.array");
    lua_pop(L, 1);

    lua_newtable(L);
    {
      set_field(L, -1, "array", impl_array);

      initialize_json_parse(L);
    }
    lua_setfield(L, -2, "json");
  }
}
