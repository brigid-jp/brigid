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

      void impl_gc(lua_State* L) {
        check_stopwatch(L, 1)->~stopwatch();
      }

      void impl_call(lua_State* L) {
        if (stopwatch* self = new_stopwatch_platform(L, nullptr)) {
          // noop
        } else {
          // die
        }
      }

      void impl_start(lua_State* L) {
        stopwatch* self = check_stopwatch(L, 1);
        self->start();
      }

      stopwatch* cache = nullptr;
      const void* cache_mt = nullptr;

      void impl_stop(lua_State* L) {
        check_stopwatch(L, 1)->stop();
//        if (stopwatch* self = static_cast<stopwatch*>(lua_touserdata(L, 1))) {
//          if (lua_getmetatable(L, 1)) {
//            const void* mt = lua_topointer(L, -1);
//            if (mt == cache_mt) {
//              self->stop();
//            } else {
//              luaL_getmetatable(L, "brigid.ubench.stopwatch");
//              if (lua_rawequal(L, -1, -2)) {
//                self->stop();
//              }
//            }
//          }
//        }

//        if (stopwatch* self = static_cast<stopwatch*>(lua_touserdata(L, 1))) {
//          if (self == cache) {
//            self->stop();
//          } else {
//            if (lua_getmetatable(L, 1)) {
//              luaL_getmetatable(L, "brigid.ubench.stopwatch");
//              if (lua_rawequal(L, -1, -2)) {
//                self->stop();
//                cache = self;
//              }
//            }
//          }
//        }
      }

      int impl_stop_c(lua_State* L) {
        if (stopwatch* self = static_cast<stopwatch*>(lua_touserdata(L, 1))) {
          if (self == cache) {
            self->stop();
          } else {
            if (lua_getmetatable(L, 1)) {
              const void* mt = lua_topointer(L, -1);
              if (mt == cache_mt) {
                self->stop();
                cache = self;
              } else {
                luaL_getmetatable(L, "brigid.ubench.stopwatch");
                if (lua_rawequal(L, -1, -2)) {
                  self->stop();
                  cache = self;
                }
              }
            }
          }
        }


//        if (stopwatch* self = static_cast<stopwatch*>(lua_touserdata(L, 1))) {
//          if (self == cache) {
//            self->stop();
//          } else {
//            if (lua_getmetatable(L, 1)) {
//              luaL_getmetatable(L, "brigid.ubench.stopwatch");
//              if (lua_rawequal(L, -1, -2)) {
//                self->stop();
//                cache = self;
//              }
//            }
//          }
//        }
        return 0;
      }

      void impl_get_elapsed(lua_State* L) {
        stopwatch* self = check_stopwatch(L, 1);
        push_integer(L, self->get_elapsed());
      }

      void initialize(lua_State* L) {
        lua_newtable(L);
        {
          set_field(L, -1, "check_runtime", impl_check_runtime);

          lua_newtable(L);
          {
            new_metatable(L, "brigid.ubench.stopwatch");
            lua_pushvalue(L, -2);
            lua_setfield(L, -2, "__index");
            set_field(L, -1, "__gc", impl_gc);

            cache_mt = lua_topointer(L, -1);

            lua_pop(L, 1);

            set_metafield(L, -1, "__call", impl_call);
            set_field(L, -1, "start", impl_start);

            set_field(L, -1, "stop", impl_stop);
            // lua_pushcfunction(L, impl_stop_c);
            // lua_setfield(L, -2, "stop");

            set_field(L, -1, "get_elapsed", impl_get_elapsed);
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
