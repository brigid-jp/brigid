// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <lua.hpp>

#include <string.h>
#include <chrono>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace brigid {
  namespace {
    std::mutex mutex;

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

    int push_48bit_lightuserdata(lua_State* L) {
      lua_pushlightuserdata(L, reinterpret_cast<void*>(0x800000000000));
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

    int nifty_count = 0;

    struct nifty_counter {
      nifty_counter() {
        ++nifty_count;
        // std::cout << "nifty_count cstr " << nifty_count << " " << std::this_thread::get_id() << "\n";
      }

      ~nifty_counter() {
        --nifty_count;
        // std::cout << "nifty_count dstr " << nifty_count << " " << std::this_thread::get_id() << "\n";
      }
    };

    namespace {
      nifty_counter nc;
    }

    void start_routine(const char* data, size_t size) {
      lua_State* L = luaL_newstate();
      luaL_openlibs(L);

      // std::cout << "[" << std::string(data, size) << "]\n";
      int result = luaL_loadbuffer(L, data, size, nullptr);
      // std::cout << "result " << result << "\n";
      if (result == 0) {
        std::ostringstream out;
        out << std::this_thread::get_id();
        std::string id = out.str();
        lua_pushlstring(L, id.data(), id.size());
        int result = lua_pcall(L, 1, 0, 0);
        if (result != 0) {
          std::cerr << "error " << lua_tostring(L, -1) << "\n";
        }
      }
      lua_close(L);
    }

    int create_native_threads(lua_State* L) {
      lua_Integer num_threads = luaL_checkinteger(L, 1);
      size_t size = 0;
      const char* data = luaL_checklstring(L, 2, &size);

      std::vector<std::thread> threads;
      for (lua_Integer i = 0; i < num_threads; ++i) {
        threads.emplace_back(std::thread(start_routine, data, size));
      }
      for (auto& thread : threads) {
        thread.join();
      }

      return 0;
    }

    std::vector<char> buffer(256);

    int get_handle_lightuserdata(lua_State* L) {
      lua_pushlightuserdata(L, buffer.data());
      return 1;
    }

    int get_handle_string(lua_State* L) {
      const void* data = buffer.data();
      static constexpr size_t size = sizeof(data);
      char buffer[size] = {};
      memcpy(buffer, &data, size);
      lua_pushlstring(L, buffer, size);
      return 1;
    }

    int dump_handle(lua_State* L) {
      if (const void* userdata = lua_touserdata(L, 1)) {
        std::cout << "userdata: " << userdata << "\n";
        return 0;
      }
      size_t size = 0;
      if (const char* data = lua_tolstring(L, 1, &size)) {
        if (sizeof(void*) == size) {
          void* ptr = nullptr;
          memcpy(&ptr, data, size);
          std::cout << "string: " << ptr << "\n";
          return 0;
        }
      }
      std::cout << "unknown\n";
      return 0;
    }

    void initialize(lua_State* L) {
      lua_pushinteger(L, sizeof(void*));
      lua_setfield(L, -2, "sizeof_void_pointer");

      lua_pushcfunction(L, string_to_lightuserdata);
      lua_setfield(L, -2, "string_to_lightuserdata");

      lua_pushcfunction(L, check_full_range_lightuserdata);
      lua_setfield(L, -2, "check_full_range_lightuserdata");

      lua_pushcfunction(L, push_48bit_lightuserdata);
      lua_setfield(L, -2, "push_48bit_lightuserdata");

      lua_pushcfunction(L, bench_get_registry01);
      lua_setfield(L, -2, "bench_get_registry01");
      lua_pushcfunction(L, bench_get_registry02);
      lua_setfield(L, -2, "bench_get_registry02");
      lua_pushcfunction(L, bench_get_registry03);
      lua_setfield(L, -2, "bench_get_registry03");

      lua_pushcfunction(L, create_native_threads);
      lua_setfield(L, -2, "create_native_threads");

      lua_pushcfunction(L, get_handle_lightuserdata);
      lua_setfield(L, -2, "get_handle_lightuserdata");
      lua_pushcfunction(L, get_handle_string);
      lua_setfield(L, -2, "get_handle_string");
      lua_pushcfunction(L, dump_handle);
      lua_setfield(L, -2, "dump_handle");
    }
  }
}

extern "C" int luaopen_test_address_space(lua_State* L) {
  std::lock_guard<std::mutex> lock(brigid::mutex);

  // std::cout << "open start " << std::this_thread::get_id() << std::endl;
  lua_newtable(L);
  brigid::initialize(L);
  // std::cout << "open ended " << std::this_thread::get_id() << std::endl;
  return 1;
}
