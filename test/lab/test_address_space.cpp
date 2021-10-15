// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <lua.hpp>

#include <string.h>
#include <chrono>
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

    int check_full_range_lightuserdata(lua_State* L) {
      void* ptr = nullptr;
      memset(&ptr, 0xFF, sizeof(ptr));
      lua_pushlightuserdata(L, ptr);
      return 1;
    }

    int bench_get_registry01(lua_State* L) {
      using clock_type = std::chrono::high_resolution_clock;

      lua_pushinteger(L, 1);
      lua_setfield(L, LUA_REGISTRYINDEX, "brigid.flag");

      lua_Integer v = 0;

      clock_type::time_point t0 = clock_type::now();
      for (size_t i = 0; i < 1000000; ++i) {
        lua_getfield(L, LUA_REGISTRYINDEX, "brigid.flag");
        v += lua_tointeger(L, -1);
        lua_pop(L, 1);
      }
      clock_type::time_point t1 = clock_type::now();
      std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count() << "\n";
      return 0;
    }

    int bench_get_registry02(lua_State* L) {
      using clock_type = std::chrono::high_resolution_clock;

      lua_pushinteger(L, 1);
      lua_rawseti(L, LUA_REGISTRYINDEX, 987654);

      lua_Integer v = 0;

      clock_type::time_point t0 = clock_type::now();
      for (size_t i = 0; i < 1000000; ++i) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, 987654);
        v += lua_tointeger(L, -1);
        lua_pop(L, 1);
      }
      clock_type::time_point t1 = clock_type::now();
      std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count() << "\n";
      return 0;
    }

    int bench_get_registry03(lua_State* L) {
      using clock_type = std::chrono::high_resolution_clock;

      lua_pushinteger(L, 1);
      lua_setfield(L, LUA_REGISTRYINDEX, "brigid.flag2");

      lua_Integer v = 0;

      clock_type::time_point t0 = clock_type::now();
      for (size_t i = 0; i < 1000000; ++i) {
        lua_pushstring(L, "brigid.flag2");
        lua_rawget(L, LUA_REGISTRYINDEX);
        v += lua_tointeger(L, -1);
        lua_pop(L, 1);
      }
      clock_type::time_point t1 = clock_type::now();
      std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count() << "\n";
      return 0;
    }

    void initialize(lua_State* L) {
      lua_pushinteger(L, sizeof(void*));
      lua_setfield(L, -2, "sizeof_void_pointer");

      lua_pushcfunction(L, string_to_lightuserdata);
      lua_setfield(L, -2, "string_to_lightuserdata");

      lua_pushcfunction(L, check_full_range_lightuserdata);
      lua_setfield(L, -2, "check_full_range_lightuserdata");

      lua_pushcfunction(L, bench_get_registry01);
      lua_setfield(L, -2, "bench_get_registry01");
      lua_pushcfunction(L, bench_get_registry02);
      lua_setfield(L, -2, "bench_get_registry02");
      lua_pushcfunction(L, bench_get_registry03);
      lua_setfield(L, -2, "bench_get_registry03");
    }
  }
}

extern "C" int luaopen_test_address_space(lua_State* L) {
  lua_newtable(L);
  brigid::initialize(L);
  return 1;
}
