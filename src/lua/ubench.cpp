// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/error.hpp>
#include <brigid/noncopyable.hpp>
#include "common.hpp"
#include "ubench.hpp"

#include <lua.hpp>

#include <stdint.h>
#include <time.h>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace brigid {
  namespace ubench {
    namespace {
      template <class T>
      void check_chrono(std::ostream& out, const char* name) {
        using std::chrono::duration_cast;
        using std::chrono::nanoseconds;

        int64_t count;
        typename T::duration duration;
        typename T::time_point t;
        typename T::time_point u;

        t = T::now();
        for (count = 1; ; ++count) {
          u = T::now();
          duration = u - t;
          if (duration > T::duration::zero()) {
            break;
          }
        }
        t = u;
        for (count = 1; ; ++count) {
          u = T::now();
          duration = u - t;
          if (duration > T::duration::zero()) {
            break;
          }
        }

        out
          << "check_chrono: " << name << "\n"
          << "  count: " << count << "\n"
          << "  duration: " << duration_cast<nanoseconds>(duration).count() << "\n"
          << "  t: " << duration_cast<nanoseconds>(t.time_since_epoch()).count() << "\n"
          << "  u: " << duration_cast<nanoseconds>(u.time_since_epoch()).count() << "\n";
      }

      void check_chrono(std::ostream& out) {
        check_chrono<std::chrono::system_clock>(out, "std::chrono::system_clock");
        check_chrono<std::chrono::steady_clock>(out, "std::chrono::steady_clock");
        check_chrono<std::chrono::high_resolution_clock>(out, "std::chrono::high_resolution_clock");
      }

      stopwatch* check_stopwatch(lua_State* L, int arg) {
        return check_udata<stopwatch>(L, arg, "brigid.ubench.stopwatch");
      }

      void impl_check_runtime(lua_State* L) {
        std::ostringstream out;
        check_platform(out);
        check_chrono(out);
        std::string result = out.str();
        lua_pushlstring(L, result.data(), result.size());
      }

      void impl_get_stopwatch_impl_names(lua_State* L) {
        lua_newtable(L);
        int i = get_stopwatch_impl_names(L, 0);

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
        if (stopwatch* self = new_stopwatch(L, nullptr)) {
          // noop
        } else {
          // die
        }
      }

      void impl_start(lua_State* L) {
        check_stopwatch(L, 1)->start();
      }

      void impl_stop(lua_State* L) {
        check_stopwatch(L, 1)->stop();
      }

      void impl_get_elapsed(lua_State* L) {
        stopwatch* self = check_stopwatch(L, 1);
        push_integer(L, self->get_elapsed());
      }

      void impl_get_impl_name(lua_State* L) {
        stopwatch* self = check_stopwatch(L, 1);
        lua_pushstring(L, self->get_impl_name());
      }

      void initialize(lua_State* L) {
        lua_newtable(L);
        {
          set_field(L, -1, "check_runtime", impl_check_runtime);
          set_field(L, -1, "get_stopwatch_impl_names", impl_get_stopwatch_impl_names);

          lua_newtable(L);
          {
            new_metatable(L, "brigid.ubench.stopwatch");
            lua_pushvalue(L, -2);
            lua_setfield(L, -2, "__index");
            set_field(L, -1, "__gc", impl_gc);
            lua_pop(L, 1);

            set_metafield(L, -1, "__call", impl_call);
            set_field(L, -1, "start", impl_start);
            set_field(L, -1, "stop", impl_stop);
            set_field(L, -1, "get_elapsed", impl_get_elapsed);
            set_field(L, -1, "get_impl_name", impl_get_impl_name);
          }
          lua_setfield(L, -2, "stopwatch");
        }
        lua_setfield(L, -2, "ubench");
      }
    }

    stopwatch::~stopwatch() {}
  }

  void initialize_ubench(lua_State* L) {
    ubench::initialize(L);
  }
}
