// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <lua.hpp>

#include <string.h>
#include <iostream>

namespace brigid {
  namespace {
    int string_to_lightuserdata(lua_State* L) {
      size_t size = 0;
      if (const char* data = luaL_checklstring(L, 1, &size)) {
        if (sizeof(void*) <= size) {
          void* ptr = nullptr;
          memcpy(&ptr, data, sizeof(void*));
          lua_pushlightuserdata(L, ptr);
          return 1;
        }
      }
      return 0;
    }

    void initialize(lua_State* L) {
      lua_pushinteger(L, sizeof(void*));
      lua_setfield(L, -2, "sizeof_void_pointer");

      lua_pushcfunction(L, string_to_lightuserdata);
      lua_setfield(L, -2, "string_to_lightuserdata");
    }
  }
}

extern "C" int luaopen_test_address_space(lua_State* L) {
  lua_newtable(L);
  brigid::initialize(L);
  return 1;
}
