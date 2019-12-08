// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "util_lua.hpp"

#include <lua.hpp>

#include <exception>

namespace brigid {
  namespace lua {
    namespace {
      int impl_closure(lua_State* L) {
        int top = lua_gettop(L);
        try {
          cxx_function_t function = reinterpret_cast<cxx_function_t>(lua_touserdata(L, lua_upvalueindex(1)));
          function(L);
        } catch (const std::exception& e) {
          lua_settop(L, top);
          return luaL_error(L, "caught exception: %s", e.what());
        }
        return lua_gettop(L) - top;
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

    void set_metatable(lua_State* L, const char* name) {
#if LUA_VERSION_NUM >= 502
      luaL_setmetatable(L, name);
#else
      luaL_getmetatable(L, name);
      lua_setmetatable(L, -2);
#endif
    }

    void push(lua_State* L, const char* value) {
      lua_pushstring(L, value);
    }

    void push(lua_State* L, const char* data, size_t size) {
      lua_pushlstring(L, data, size);
    }

    void push(lua_State* L, cxx_function_t value) {
      lua_pushlightuserdata(L, reinterpret_cast<void*>(value));
      lua_pushcclosure(L, impl_closure, 1);
    }
  }
}
