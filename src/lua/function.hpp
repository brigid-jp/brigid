// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_FUNCTION_HPP
#define BRIGID_FUNCTION_HPP

#include <lua.hpp>

#include <exception>
#include <stdexcept>

namespace brigid {
  namespace detail {
    void set_field(lua_State*, int, const char*, lua_CFunction);
    void set_metafield(lua_State*, int, const char*, lua_CFunction);
  }

  template <class T, T (*)(lua_State*)>
  struct function_wrapper;

  template <int (*T)(lua_State*)>
  struct function_wrapper<int, T> {
    static int value(lua_State* L) {
      try {
        return T(L);
      } catch (const std::runtime_error& e) {
        lua_pushnil(L);
        lua_pushstring(L, e.what());
        return 2;
      } catch (const std::exception& e) {
        return luaL_error(L, "%s", e.what());
      }
    }

    static void set_field(lua_State* L, int index, const char* key) {
      detail::set_field(L, index, key, value);
    }

    static void set_metafield(lua_State* L, int index, const char* key) {
      detail::set_metafield(L, index, key, value);
    }
  };

  template <void (*T)(lua_State*)>
  struct function_wrapper<void, T> {
    static int value(lua_State* L) {
      try {
        int top = lua_gettop(L);
        T(L);
        int result = lua_gettop(L) - top;
        if (result > 0) {
          return result;
        } else {
          if (lua_toboolean(L, 1)) {
            lua_pushvalue(L, 1);
          } else {
            lua_pushboolean(L, true);
          }
          return 1;
        }
      } catch (const std::runtime_error& e) {
        lua_pushnil(L);
        lua_pushstring(L, e.what());
        return 2;
      } catch (const std::exception& e) {
        return luaL_error(L, "%s", e.what());
      }
    }

    static void set_field(lua_State* L, int index, const char* key) {
      detail::set_field(L, index, key, value);
    }

    static void set_metafield(lua_State* L, int index, const char* key) {
      detail::set_metafield(L, index, key, value);
    }
  };

  template <int (*T)(lua_State*)>
  function_wrapper<int, T> function();

  template <void (*T)(lua_State*)>
  function_wrapper<void, T> function();
}

#endif
