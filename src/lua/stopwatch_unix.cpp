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
#include <time.h>

namespace brigid {
  namespace {
    char NAME_CLOCK_REALTIME[] = "CLOCK_REALTIME";
#ifdef CLOCK_REALTIME_COARSE
    char NAME_CLOCK_REALTIME_COARSE[] = "CLOCK_REALTIME_COARSE";
#endif
    char NAME_CLOCK_MONOTONIC[] = "CLOCK_MONOTONIC";
#ifdef CLOCK_MONOTONIC_COARSE
    char NAME_CLOCK_MONOTONIC_COARSE[] = "CLOCK_MONOTONIC_COARSE";
#endif
#ifdef CLOCK_MONOTONIC_RAW
    char NAME_CLOCK_MONOTONIC_RAW[] = "CLOCK_MONOTONIC_RAW";
#endif
#ifdef CLOCK_MONOTONIC_RAW_APPROX
    char NAME_CLOCK_MONOTONIC_RAW_APPROX[] = "CLOCK_MONOTONIC_RAW_APPROX";
#endif
#ifdef CLOCK_BOOTTIME
    char NAME_CLOCK_BOOTTIME[] = "CLOCK_BOOTTIME";
#endif
#ifdef CLOCK_UPTIME_RAW
    char NAME_CLOCK_UPTIME_RAW[] = "CLOCK_UPTIME_RAW";
#endif
#ifdef CLOCK_UPTIME_RAW_APPROX
    char NAME_CLOCK_UPTIME_RAW_APPROX[] = "CLOCK_UPTIME_RAW_APPROX";
#endif

    template <clockid_t T_clock, const char* T_name>
    class stopwatch_unix : public stopwatch, private noncopyable {
    public:
      stopwatch_unix()
        : started_(),
          stopped_() {}

      virtual const char* get_name() const {
        return T_name;
      }

      virtual void start() {
        if (clock_gettime(T_clock, &started_) == -1) {
          throw BRIGID_SYSTEM_ERROR();
        }
      }

      virtual void stop() {
        if (clock_gettime(T_clock, &stopped_) == -1) {
          throw BRIGID_SYSTEM_ERROR();
        }
      }

      virtual int64_t get_elapsed() const {
        return (stopped_.tv_sec - started_.tv_sec) * 1000000000LL + stopped_.tv_nsec - started_.tv_nsec;
      }

    private:
      struct timespec started_;
      struct timespec stopped_;
    };

    template <clockid_t T_clock, const char* T_name>
    stopwatch* new_stopwatch_unix(lua_State* L) {
      return new_userdata<stopwatch_unix<T_clock, T_name> >(L, "brigid.stopwatch");
    }
  }

  stopwatch* new_stopwatch(lua_State* L) {
#ifdef CLOCK_MONOTONIC_RAW
    return new_stopwatch_unix<CLOCK_MONOTONIC_RAW, NAME_CLOCK_MONOTONIC_RAW>(L);
#else
    return new_stopwatch_unix<CLOCK_MONOTONIC, NAME_CLOCK_MONOTONIC>(L);
#endif
  }

  stopwatch* new_stopwatch(lua_State* L, const char* name) {
    if (strcmp(name, NAME_CLOCK_REALTIME) == 0) {
      return new_stopwatch_unix<CLOCK_REALTIME, NAME_CLOCK_REALTIME>(L);
    }

#ifdef CLOCK_REALTIME_COARSE
    if (strcmp(name, NAME_CLOCK_REALTIME_COARSE) == 0) {
      return new_stopwatch_unix<CLOCK_REALTIME_COARSE, NAME_CLOCK_REALTIME_COARSE>(L);
    }
#endif

    if (strcmp(name, NAME_CLOCK_MONOTONIC) == 0) {
      return new_stopwatch_unix<CLOCK_MONOTONIC, NAME_CLOCK_MONOTONIC>(L);
    }

#ifdef CLOCK_MONOTONIC_COARSE
    if (strcmp(name, NAME_CLOCK_MONOTONIC_COARSE) == 0) {
      return new_stopwatch_unix<CLOCK_MONOTONIC_COARSE, NAME_CLOCK_MONOTONIC_COARSE>(L);
    }
#endif

#ifdef CLOCK_MONOTONIC_RAW
    if (strcmp(name, NAME_CLOCK_MONOTONIC_RAW) == 0) {
      return new_stopwatch_unix<CLOCK_MONOTONIC_RAW, NAME_CLOCK_MONOTONIC_RAW>(L);
    }
#endif

#ifdef CLOCK_MONOTONIC_RAW_APPROX
    if (strcmp(name, NAME_CLOCK_MONOTONIC_RAW_APPROX) == 0) {
      return new_stopwatch_unix<CLOCK_MONOTONIC_RAW_APPROX, NAME_CLOCK_MONOTONIC_RAW_APPROX>(L);
    }
#endif

#ifdef CLOCK_BOOTTIME
    if (strcmp(name, NAME_CLOCK_BOOTTIME) == 0) {
      return new_stopwatch_unix<CLOCK_BOOTTIME, NAME_CLOCK_BOOTTIME>(L);
    }
#endif

#ifdef CLOCK_UPTIME_RAW
    if (strcmp(name, NAME_CLOCK_UPTIME_RAW) == 0) {
      return new_stopwatch_unix<CLOCK_UPTIME_RAW, NAME_CLOCK_UPTIME_RAW>(L);
    }
#endif

#ifdef CLOCK_UPTIME_RAW_APPROX
    if (strcmp(name, NAME_CLOCK_UPTIME_RAW_APPROX) == 0) {
      return new_stopwatch_unix<CLOCK_UPTIME_RAW_APPROX, NAME_CLOCK_UPTIME_RAW_APPROX>(L);
    }
#endif

    return nullptr;
  }

  int get_stopwatch_names(lua_State* L, int i) {
    lua_pushstring(L, NAME_CLOCK_REALTIME);
    lua_rawseti(L, -2, ++i);

#ifdef CLOCK_REALTIME_COARSE
    lua_pushstring(L, NAME_CLOCK_REALTIME_COARSE);
    lua_rawseti(L, -2, ++i);
#endif

    lua_pushstring(L, NAME_CLOCK_MONOTONIC);
    lua_rawseti(L, -2, ++i);

#ifdef CLOCK_MONOTONIC_COARSE
    lua_pushstring(L, NAME_CLOCK_MONOTONIC_COARSE);
    lua_rawseti(L, -2, ++i);
#endif

#ifdef CLOCK_MONOTONIC_RAW
    lua_pushstring(L, NAME_CLOCK_MONOTONIC_RAW);
    lua_rawseti(L, -2, ++i);
#endif

#ifdef CLOCK_MONOTONIC_RAW_APPROX
    lua_pushstring(L, NAME_CLOCK_MONOTONIC_RAW_APPROX);
    lua_rawseti(L, -2, ++i);
#endif

#ifdef CLOCK_BOOTTIME
    lua_pushstring(L, NAME_CLOCK_BOOTTIME);
    lua_rawseti(L, -2, ++i);
#endif

#ifdef CLOCK_UPTIME_RAW
    lua_pushstring(L, NAME_CLOCK_UPTIME_RAW);
    lua_rawseti(L, -2, ++i);
#endif

#ifdef CLOCK_UPTIME_RAW_APPROX
    lua_pushstring(L, NAME_CLOCK_UPTIME_RAW_APPROX);
    lua_rawseti(L, -2, ++i);
#endif

    return i;
  }
}
