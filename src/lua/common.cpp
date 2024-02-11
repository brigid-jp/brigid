// Copyright (c) 2019-2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "common.hpp"
#include "error.hpp"
#include "function.hpp"
#include "stack_guard.hpp"

#include <lua.hpp>

#include <dlfcn.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <limits>
#include <mutex>

extern "C" int luaopen_brigid(lua_State*);

namespace brigid {
  namespace {
    std::once_flag once;
    int lightuserdata_bits = std::numeric_limits<uintptr_t>::digits;
    uintptr_t lightuserdata_mask = 0;

    int check_lightuserdata(lua_State* L) {
      int bits = std::numeric_limits<uintptr_t>::digits - check_integer<int>(L, 1);
      uintptr_t v = static_cast<uintptr_t>(-1) >> bits;
      lua_pushlightuserdata(L, reinterpret_cast<void*>(v));
      return 1;
    }

    void bootstrap(lua_State* L) {
      for (int i = 1; i <= lightuserdata_bits; ++i) {
        stack_guard guard(L);
        lua_pushcfunction(L, check_lightuserdata);
        lua_pushinteger(L, i);
        if (lua_pcall(L, 1, 0, 0) != 0) {
          lightuserdata_bits = i - 1;
          lightuserdata_mask = static_cast<uintptr_t>(-1) << lightuserdata_bits;
          break;
        }
      }
    }

    void impl_get_lightuserdata_bits(lua_State* L) {
      push_integer(L, lightuserdata_bits);
    }

    void impl_get_version(lua_State* L) {
      static const char version[] =
#define m4_define(_, value) #value;
#include "brigid_version.m4"
#undef m4_define
      ;
      lua_pushstring(L, version);
    }

    void impl_dladdr(lua_State* L) {
      Dl_info info = {};
      if (dladdr(reinterpret_cast<const void*>(luaopen_brigid), &info)) {
        lua_pushstring(L, info.dli_fname);
      } else {
        throw BRIGID_RUNTIME_ERROR(dlerror());
      }
    }

    void impl_dlopen(lua_State* L) {
      const char* path = luaL_checkstring(L, 1);
      if (!dlopen(path, RTLD_NOW | RTLD_GLOBAL)) {
        throw BRIGID_RUNTIME_ERROR(dlerror());
      }
    }
  }

  namespace detail {
    void push_pointer(lua_State* L, const void* source) {
      if (reinterpret_cast<uintptr_t>(source) & lightuserdata_mask) {
        static const size_t size = sizeof(source);
        char buffer[size] = {};
        memcpy(buffer, &source, size);
        lua_getfield(L, LUA_REGISTRYINDEX, "brigid.string_to_ffi_pointer");
        lua_pushlstring(L, buffer, size);
        if (lua_pcall(L, 1, 1, 0) != 0) {
          throw BRIGID_LOGIC_ERROR(lua_tostring(L, -1));
        }
      } else {
        lua_pushlightuserdata(L, const_cast<void*>(source));
      }
    }

    void* to_pointer(lua_State* L, int index) {
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
  }

  int abs_index(lua_State* L, int index) {
#if LUA_VERSION_NUM >= 502
    return lua_absindex(L, index);
#else
    if (index > 0 || index <= LUA_REGISTRYINDEX) {
      return index;
    } else {
      return lua_gettop(L) + index + 1;
    }
#endif
  }

  int get_field(lua_State* L, int index, const char* key) {
#if LUA_VERSION_NUM >= 503
    return lua_getfield(L, index, key);
#else
    lua_getfield(L, index, key);
    return lua_type(L, -1);
#endif
  }

  int new_metatable(lua_State* L, const char* name) {
#if LUA_VERSION_NUM >= 503
    return luaL_newmetatable(L, name);
#else
    if (luaL_newmetatable(L, name)) {
      lua_pushstring(L, name);
      lua_setfield(L, -2, "__name");
      return 1;
    } else {
      return 0;
    }
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

  void initialize_common(lua_State* L) {
    std::call_once(once, bootstrap, L);

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
      lua_pushboolean(L, lightuserdata_mask != 0);
      if (lua_pcall(L, 2, 0, 0) != 0) {
        throw BRIGID_LOGIC_ERROR(lua_tostring(L, -1));
      }
    }
    {
      lua_getfield(L, -1, "is_love2d_data");
      lua_setfield(L, LUA_REGISTRYINDEX, "brigid.is_love2d_data");

      if (lightuserdata_mask) {
        lua_getfield(L, -1, "string_to_ffi_pointer");
        lua_setfield(L, LUA_REGISTRYINDEX, "brigid.string_to_ffi_pointer");
      }
    }
    lua_pop(L, 1);

    lua_pushlightuserdata(L, nullptr);
    lua_setfield(L, -2, "null");

    decltype(function<impl_get_lightuserdata_bits>())::set_field(L, -1, "get_lightuserdata_bits");
    decltype(function<impl_get_version>())::set_field(L, -1, "get_version");
    decltype(function<impl_dladdr>())::set_field(L, -1, "dladdr");
    decltype(function<impl_dlopen>())::set_field(L, -1, "dlopen");
  }
}
