// Copyright (c) 2019-2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/error.hpp>
#include "common.hpp"
#include "stack_guard.hpp"

#include <lua.hpp>

#include <stddef.h>
#include <string.h>
#include <exception>
#include <mutex>
#include <stdexcept>
#include <string>

#include <iostream>

namespace brigid {
  namespace {
    std::once_flag once;

    bool no_full_range_lightuserdata;

    int check_full_range_lightuserdata(lua_State* L) {
      void* ptr = nullptr;
      memset(&ptr, 0xFF, sizeof(ptr));
      lua_pushlightuserdata(L, ptr);
      return 1;
    }

    void bootstrap(lua_State* L) {
      int top = lua_gettop(L);
      lua_pushcfunction(L, check_full_range_lightuserdata);
      no_full_range_lightuserdata = lua_pcall(L, 0, 0, 0) != 0;
      lua_settop(L, top);
    }

    int impl_closure(lua_State* L) {
      int top = lua_gettop(L);
      try {
        if (cxx_function_t function = to_handle<cxx_function_t>(L, lua_upvalueindex(1))) {
          function(L);
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
        }
      } catch (const std::runtime_error& e) {
        lua_settop(L, top);
        lua_pushnil(L);
        lua_pushstring(L, e.what());
        return 2;
      } catch (const std::exception& e) {
        lua_settop(L, top);
        return luaL_error(L, "%s", e.what());
      }
      lua_settop(L, top);
      return luaL_error(L, "attempt to call an invalid upvalue");
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

  void new_metatable(lua_State* L, const char* name) {
    luaL_newmetatable(L, name);
#if LUA_VERSION_NUM <= 502
    lua_pushstring(L, name);
    lua_setfield(L, -2, "__name");
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

  bool is_false(lua_State* L, int index) {
    return lua_isboolean(L, index) && !lua_toboolean(L, index);
  }

  void push_handle_impl(lua_State* L, const void* source) {
    if (no_full_range_lightuserdata) {
      static constexpr size_t size = sizeof(source);
      char buffer[size] = {};
      memcpy(buffer, &source, size);
      lua_pushlstring(L, buffer, size);
    } else {
      lua_pushlightuserdata(L, const_cast<void*>(source));
    }
  }

  void push_pointer_impl(lua_State* L, const void* source) {
    if (no_full_range_lightuserdata) {
      static constexpr size_t size = sizeof(source);
      char buffer[size] = {};
      memcpy(buffer, &source, size);
      lua_getfield(L, LUA_REGISTRYINDEX, "brigid.common.decode_pointer");
      lua_pushlstring(L, buffer, size);
      if (lua_pcall(L, 1, 1, 0) != 0) {
        throw BRIGID_LOGIC_ERROR(lua_tostring(L, -1));
      }
    } else {
      lua_pushlightuserdata(L, const_cast<void*>(source));
    }
  }

  void* to_handle_impl(lua_State* L, int index) {
    switch (lua_type(L, index)) {
      case LUA_TSTRING:
        {
          size_t size = 0;
          if (const char* data = lua_tolstring(L, index, &size)) {
            if (size == sizeof(void*)) {
              void* result = nullptr;
              memcpy(&result, data, size);
              return result;
            }
          }
        }
        return nullptr;
      case LUA_TLIGHTUSERDATA:
        return lua_touserdata(L, index);
      default:
        return nullptr;
    }
  }

  void set_field(lua_State* L, int index, const char* key, cxx_function_t value) {
    index = abs_index(L, index);
    push_handle(L, value);
    lua_pushcclosure(L, impl_closure, 1);
    lua_setfield(L, index, key);
  }

  void set_metafield(lua_State* L, int index, const char* key, cxx_function_t value) {
    index = abs_index(L, index);
    if (lua_getmetatable(L, index)) {
      set_field(L, -1, key, value);
      lua_pop(L, 1);
    } else {
      lua_newtable(L);
      set_field(L, -1, key, value);
      lua_setmetatable(L, index);
    }
  }

  int get_field(lua_State* L, int index, const char* key) {
#if LUA_VERSION_NUM >= 503
    return lua_getfield(L, index, key);
#else
    lua_getfield(L, index, key);
    return lua_type(L, -1);
#endif
  }

  void initialize_common(lua_State* L) {
    try {
      std::call_once(once, bootstrap, L);
    } catch (const std::exception& e) {
      luaL_error(L, "%s", e.what());
    }

    lua_newtable(L);
    {
      stack_guard guard(L);
      static const char code[] =
      #include "common.lua"
      ;
      if (luaL_loadbuffer(L, code, strlen(code), "=(load)") != 0) {
        throw BRIGID_LOGIC_ERROR(lua_tostring(L, -1));
      }
      lua_pushvalue(L, -2);
      lua_pushboolean(L, no_full_range_lightuserdata);
      if (lua_pcall(L, 2, 0, 0) != 0) {
        throw BRIGID_LOGIC_ERROR(lua_tostring(L, -1));
      }
    }
    {
      if (no_full_range_lightuserdata) {
        lua_getfield(L, -1, "decode_pointer");
        lua_setfield(L, LUA_REGISTRYINDEX, "brigid.common.decode_pointer");
      }

      lua_getfield(L, -1, "is_love2d_data");
      lua_setfield(L, LUA_REGISTRYINDEX, "brigid.common.is_love2d_data");
    }
    lua_pop(L, 1);
  }
}
