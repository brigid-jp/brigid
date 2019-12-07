// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "util_lua.hpp"

#include <lua.hpp>

#include <exception>

namespace brigid {
  namespace lua {
    namespace {
      int closure(lua_State* L) {
        int top = lua_gettop(L);
        try {
          cxx_function_t function = reinterpret_cast<cxx_function_t>(lua_touserdata(L, lua_upvalueindex(1)));
          function(L);
          return lua_gettop(L) - top;
        } catch (const std::exception& e) {
          lua_settop(L, top);
          return luaL_error(L, "caught exception: %s", e.what());
        } catch (...) {
          lua_settop(L, top);
          return luaL_error(L, "caught unknown exception");
        }
      }
    }

    int abs_index(lua_State* L, int index) {
#if LUA_VERSION_NUM >= 502
      return lua_absindex(L, index);
#else
      if (index < 0) {
        int top = lua_gettop(L);
        if (top >= -index) {
          return top + index + 1;
        }
      }
      return index;
#endif
    }

    void push(lua_State* L, cxx_function_t value) {
      lua_pushlightuserdata(L, reinterpret_cast<void*>(value));
      lua_pushcclosure(L, closure, 1);
    }
  }
}
