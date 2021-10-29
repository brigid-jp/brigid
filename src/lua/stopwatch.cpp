// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/error.hpp>
#include <brigid/noncopyable.hpp>
#include "common.hpp"
#include "stopwatch.hpp"

#include <lua.hpp>

#include <stdint.h>
#include <string.h>
#include <chrono>

namespace brigid {
  /*
  template <class T, T (*T_function)(lua_State*)>
  struct exception_handler;

  template <void (*T_function)(lua_State*)>
  struct exception_handler<void, T_function> {
    static int value(lua_State* L) {
      try {
        int top = lua_gettop(L);
        T_function(L);
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
  };

  template <int (*T_function)(lua_State*)>
  struct exception_handler<int, T_function> {
    static int value(lua_State* L) {
      try {
        return T_function(L);
      } catch (const std::runtime_error& e) {
        lua_pushnil(L);
        lua_pushstring(L, e.what());
        return 2;
      } catch (const std::exception& e) {
        return luaL_error(L, "%s", e.what());
      }
    }
  };

  template <void_function_t T_function>
  struct closure_noeh {
    static int value(lua_State* L) {
      T_function(L);
      return 0;
    }
  };
  */

  namespace {
    static const char* names[] = {
      "std::chrono::system_clock",          // [0]
      "std::chrono::steady_clock",          // [1]
      "std::chrono::high_resolution_clock", // [2]
    };

    template <class T, int T_name>
    class stopwatch_chrono : public stopwatch, private noncopyable {
    public:
      virtual void start() {
        started_ = T::now();
      }

      virtual void stop() {
        stopped_ = T::now();
      }

      virtual int64_t get_elapsed() const {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(stopped_ - started_).count();
      }

      virtual const char* get_name() const {
        return names[T_name];
      }

      virtual double get_resolution() const {
        return 0;
      }

    private:
      typename T::time_point started_;
      typename T::time_point stopped_;
    };

    template <class T, int T_name>
    stopwatch* new_stopwatch_chrono(lua_State* L) {
      return new_userdata<stopwatch_chrono<T, T_name> >(L, "brigid.stopwatch");
    }

    stopwatch* new_stopwatch_chrono(lua_State* L, const char* name) {
      if (!name) {
        return new_stopwatch_chrono<std::chrono::steady_clock, 1>(L);
      }

      if (strcmp(name, "std::chrono::system_clock") == 0) {
        return new_stopwatch_chrono<std::chrono::system_clock, 0>(L);
      }
      if (strcmp(name, "std::chrono::steady_clock") == 0) {
        return new_stopwatch_chrono<std::chrono::steady_clock, 1>(L);
      }
      if (strcmp(name, "std::chrono::high_resolution_clock") == 0) {
        return new_stopwatch_chrono<std::chrono::high_resolution_clock, 2>(L);
      }

      return nullptr;
    }

    stopwatch* check_stopwatch(lua_State* L, int arg) {
      return check_udata<stopwatch>(L, arg, "brigid.stopwatch");
    }

    void impl_get_stopwatch_names(lua_State* L) {
      lua_newtable(L);
      int i = get_stopwatch_names(L, 0);

      lua_pushstring(L, "std::chrono::system_clock");
      lua_rawseti(L, -2, ++i);
      lua_pushstring(L, "std::chrono::steady_clock");
      lua_rawseti(L, -2, ++i);
      lua_pushstring(L, "std::chrono::high_resolution_clock");
      lua_rawseti(L, -2, ++i);
    }

    void impl_gc(lua_State* L) {
      check_stopwatch(L, 1)->~stopwatch();
    }

    void impl_call(lua_State* L) {
      const char* name = lua_tostring(L, 2);
      if (!new_stopwatch(L, name)) {
        if (!new_stopwatch_chrono(L, name)) {
          luaL_argerror(L, 2, "unsupported stopwatch name");
        }
      }
    }

    void impl_start(lua_State* L) {
      stopwatch* self = check_stopwatch(L, 1);
      self->start();
    }

    void impl_stop(lua_State* L) {
      stopwatch* self = check_stopwatch(L, 1);
      self->stop();
    }

    void impl_get_elapsed(lua_State* L) {
      stopwatch* self = check_stopwatch(L, 1);
      push_integer(L, self->get_elapsed());
    }

    void impl_get_name(lua_State* L) {
      stopwatch* self = check_stopwatch(L, 1);
      lua_pushstring(L, self->get_name());
    }

    void impl_get_resolution(lua_State* L) {
      stopwatch* self = check_stopwatch(L, 1);
      lua_pushnumber(L, self->get_resolution());
    }

    int impl_pcall(lua_State* L) {
      stopwatch* self = check_stopwatch(L, 1);
      self->start();
      int result = lua_pcall(L, lua_gettop(L) - 2, LUA_MULTRET, 0);
      self->stop();
      if (result == 0) {
        lua_pushboolean(L, true);
        lua_replace(L, 1);
        return lua_gettop(L);
      } else {
        lua_pushboolean(L, false);
        lua_replace(L, 1);
        return 2;
      }
    }
  }

  stopwatch::~stopwatch() {}

  void initialize_stopwatch(lua_State* L) {
    set_field(L, -1, "get_stopwatch_names", impl_get_stopwatch_names);

    lua_newtable(L);
    {
      new_metatable(L, "brigid.stopwatch");
      lua_pushvalue(L, -2);
      lua_setfield(L, -2, "__index");
      set_field(L, -1, "__gc", impl_gc);
      lua_pop(L, 1);

      set_metafield(L, -1, "__call", impl_call);
      set_field(L, -1, "start", impl_start);
      set_field(L, -1, "stop", impl_stop);
      set_field(L, -1, "get_elapsed", impl_get_elapsed);
      set_field(L, -1, "get_name", impl_get_name);
      set_field(L, -1, "get_resolution", impl_get_resolution);
      set_field(L, -1, "pcall", impl_pcall);
    }
    lua_setfield(L, -2, "stopwatch");
  }
}
