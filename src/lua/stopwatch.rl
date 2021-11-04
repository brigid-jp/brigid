// vim: syntax=ragel:

// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "common.hpp"
#include "error.hpp"
#include "function.hpp"
#include "noncopyable.hpp"
#include "stopwatch.hpp"

#include <lua.hpp>

#include <stdint.h>
#include <chrono>

namespace brigid {
  namespace {
    char NAME_SYSTEM_CLOCK[] = "std::chrono::system_clock";
    char NAME_STEADY_CLOCK[] = "std::chrono::steady_clock";
    char NAME_HIGH_RESOLUTION_CLOCK[] = "std::chrono::high_resolution_clock";

    template <class T, const char* T_name>
    class stopwatch_chrono : public stopwatch, private noncopyable {
    public:
      virtual const char* get_name() const {
        return T_name;
      }

      virtual void start() {
        started_ = T::now();
      }

      virtual void stop() {
        stopped_ = T::now();
      }

      virtual int64_t get_elapsed() const {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(stopped_ - started_).count();
      }

    private:
      typename T::time_point started_;
      typename T::time_point stopped_;
    };

    template <class T, const char* T_name>
    stopwatch* new_stopwatch_chrono(lua_State* L) {
      return new_userdata<stopwatch_chrono<T, T_name> >(L, "brigid.stopwatch");
    }

    %%{
      machine stopwatch_name_chooser;

      main :=
        ( "std::chrono::system_clock\0"
          @{ return new_stopwatch_chrono<std::chrono::system_clock, NAME_SYSTEM_CLOCK>(L); }
        | "std::chrono::steady_clock\0"
          @{ return new_stopwatch_chrono<std::chrono::steady_clock, NAME_STEADY_CLOCK>(L); }
        | "std::chrono::high_resolution_clock\0"
          @{ return new_stopwatch_chrono<std::chrono::high_resolution_clock, NAME_HIGH_RESOLUTION_CLOCK>(L); }
        );

      write data noerror nofinal noentry;
    }%%

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#endif

    stopwatch* new_stopwatch_chrono(lua_State* L, const char* name) {
      int cs = 0;
      %%write init;
      const char* p = name;
      const char* pe = nullptr;
      %%write exec;
      return nullptr;
    }

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

    stopwatch* check_stopwatch(lua_State* L, int arg) {
      return check_udata<stopwatch>(L, arg, "brigid.stopwatch");
    }

    void impl_get_stopwatch_names(lua_State* L) {
      lua_newtable(L);
      int i = get_stopwatch_names(L, 0);

      lua_pushstring(L, NAME_SYSTEM_CLOCK);
      lua_rawseti(L, -2, ++i);
      lua_pushstring(L, NAME_STEADY_CLOCK);
      lua_rawseti(L, -2, ++i);
      lua_pushstring(L, NAME_HIGH_RESOLUTION_CLOCK);
      lua_rawseti(L, -2, ++i);
    }

    void impl_gc(lua_State* L) {
      stopwatch* self = check_stopwatch(L, 1);
      self->~stopwatch();
    }

    void impl_call(lua_State* L) {
      const char* name = lua_tostring(L, 2);
      if (name) {
        if (!new_stopwatch(L, name) && !new_stopwatch_chrono(L, name)) {
          luaL_argerror(L, 2, "unsupported stopwatch");
        }
      } else {
        new_stopwatch(L);
      }
    }

    void impl_get_name(lua_State* L) {
      stopwatch* self = check_stopwatch(L, 1);
      lua_pushstring(L, self->get_name());
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
    decltype(function<impl_get_stopwatch_names>())::set_field(L, -1, "get_stopwatch_names");

    lua_newtable(L);
    {
      new_metatable(L, "brigid.stopwatch");
      lua_pushvalue(L, -2);
      lua_setfield(L, -2, "__index");
      decltype(function<impl_gc>())::set_field(L, -1, "__gc");
      lua_pop(L, 1);

      decltype(function<impl_call>())::set_metafield(L, -1, "__call");
      decltype(function<impl_get_name>())::set_field(L, -1, "get_name");
      decltype(function<impl_start>())::set_field(L, -1, "start");
      decltype(function<impl_stop>())::set_field(L, -1, "stop");
      decltype(function<impl_get_elapsed>())::set_field(L, -1, "get_elapsed");
      decltype(function<impl_pcall>())::set_field(L, -1, "pcall");
    }
    lua_setfield(L, -2, "stopwatch");
  }
}
