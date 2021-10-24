// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/error.hpp>
#include <brigid/noncopyable.hpp>
#include "common.hpp"

#include <lua.hpp>

#include <time.h>

namespace brigid {
  namespace {
    static const clockid_t clock = CLOCK_MONOTONIC;

    class stopwatch : private noncopyable {
    public:
      stopwatch()
        : started_(),
          stopped_() {}

      void start() {
        if (clock_gettime(clock, &started_) == -1) {
          throw BRIGID_SYSTEM_ERROR();
        }
      }

      void stop() {
        if (clock_gettime(clock, &stopped_) == -1) {
          throw BRIGID_SYSTEM_ERROR();
        }
      }

      int64_t get_elapsed() const {
        int64_t sec = stopped_.tv_sec - started_.tv_sec;
        int32_t nsec = stopped_.tv_nsec - started_.tv_nsec;
        if (nsec < 0) {
          --sec;
          nsec += 1000000000;
        }
        return sec * 1000000000 + nsec;
      }

    private:
      struct timespec started_;
      struct timespec stopped_;
    };

    stopwatch* check_stopwatch(lua_State* L, int arg) {
      return check_udata<stopwatch>(L, arg, "brigid.ubench.stopwatch");
    }

    void impl_call(lua_State* L) {
      new_userdata<stopwatch>(L, "brigid.ubench.stopwatch");
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
  }

  void initialize_ubench(lua_State* L) {
    lua_newtable(L);
    {
      lua_newtable(L);
      {
        new_metatable(L, "brigid.ubench.stopwatch");
        lua_pushvalue(L, -2);
        lua_setfield(L, -2, "__index");
        lua_pop(L, 1);

        set_metafield(L, -1, "__call", impl_call);
        set_field(L, -1, "start", impl_start);
        set_field(L, -1, "stop", impl_stop);
        set_field(L, -1, "get_elapsed", impl_get_elapsed);
      }
      lua_setfield(L, -2, "stopwatch");
    }
    lua_setfield(L, -2, "ubench");
  }
}
