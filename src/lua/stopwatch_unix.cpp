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
    static const char* names[] = {
      "CLOCK_REALTIME",             // [0]
      "CLOCK_REALTIME_COARSE",      // [1] linux
      "CLOCK_MONOTONIC",            // [2]
      "CLOCK_MONOTONIC_COARSE",     // [3] linux
      "CLOCK_MONOTONIC_RAW",        // [4] linux or apple
      "CLOCK_MONOTONIC_RAW_APPROX", // [5] apple
      "CLOCK_BOOTTIME",             // [6] linux
      "CLOCK_UPTIME_RAW",           // [7] apple
      "CLOCK_UPTIME_RAW_APPROX",    // [8] apple
    };

    template <clockid_t T_clock, int T_name>
    class stopwatch_unix : public stopwatch, private noncopyable {
    public:
      stopwatch_unix()
        : started_(),
          stopped_() {}

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

      virtual const char* get_name() const {
        return names[T_name];
      }

      virtual double get_resolution() const {
        struct timespec resolution = {};
        if (clock_getres(T_clock, &resolution) == -1) {
          throw BRIGID_SYSTEM_ERROR();
        }
        return resolution.tv_sec * 1000000000LL + resolution.tv_nsec;
      }

    private:
      struct timespec started_;
      struct timespec stopped_;
    };

    template <clockid_t T_clock, int T_name>
    stopwatch* new_stopwatch_unix(lua_State* L) {
      return new_userdata<stopwatch_unix<T_clock, T_name> >(L, "brigid.stopwatch");
    }
  }

  stopwatch* new_stopwatch(lua_State* L, const char* name) {
    if (!name) {
#ifdef CLOCK_MONOTONIC_RAW
      return new_stopwatch_unix<CLOCK_MONOTONIC_RAW, 4>(L);
#else
      return new_stopwatch_unix<CLOCK_MONOTONIC, 2>(L);
#endif
    }

    if (strcasecmp(name, "CLOCK_REALTIME") == 0) {
      return new_stopwatch_unix<CLOCK_REALTIME, 0>(L);
    }

#ifdef CLOCK_REALTIME_COARSE
    if (strcasecmp(name, "CLOCK_REALTIME_COARSE") == 0) {
      return new_stopwatch_unix<CLOCK_REALTIME_COARSE, 1>(L);
    }
#endif

    if (strcasecmp(name, "CLOCK_MONOTONIC") == 0) {
      return new_stopwatch_unix<CLOCK_MONOTONIC, 2>(L);
    }

#ifdef CLOCK_MONOTONIC_COARSE
    if (strcasecmp(name, "CLOCK_MONOTONIC_COARSE") == 0) {
      return new_stopwatch_unix<CLOCK_MONOTONIC_COARSE, 3>(L);
    }
#endif

#ifdef CLOCK_MONOTONIC_RAW
    if (strcasecmp(name, "CLOCK_MONOTONIC_RAW") == 0) {
      return new_stopwatch_unix<CLOCK_MONOTONIC_RAW, 4>(L);
    }
#endif

#ifdef CLOCK_MONOTONIC_RAW_APPROX
    if (strcasecmp(name, "CLOCK_MONOTONIC_RAW_APPROX") == 0) {
      return new_stopwatch_unix<CLOCK_MONOTONIC_RAW_APPROX, 5>(L);
    }
#endif

#ifdef CLOCK_BOOTTIME
    if (strcasecmp(name, "CLOCK_BOOTTIME") == 0) {
      return new_stopwatch_unix<CLOCK_BOOTTIME, 6>(L);
    }
#endif

#ifdef CLOCK_UPTIME_RAW
    if (strcasecmp(name, "CLOCK_UPTIME_RAW") == 0) {
      return new_stopwatch_unix<CLOCK_UPTIME_RAW, 7>(L);
    }
#endif

#ifdef CLOCK_UPTIME_RAW_APPROX
    if (strcasecmp(name, "CLOCK_UPTIME_RAW_APPROX") == 0) {
      return new_stopwatch_unix<CLOCK_UPTIME_RAW_APPROX, 8>(L);
    }
#endif

    return nullptr;
  }

  int get_stopwatch_names(lua_State* L, int i) {
    lua_pushstring(L, "CLOCK_REALTIME");
    lua_rawseti(L, -2, ++i);

#ifdef CLOCK_REALTIME_COARSE
    lua_pushstring(L, "CLOCK_REALTIME_COARSE");
    lua_rawseti(L, -2, ++i);
#endif

    lua_pushstring(L, "CLOCK_MONOTONIC");
    lua_rawseti(L, -2, ++i);

#ifdef CLOCK_MONOTONIC_COARSE
    lua_pushstring(L, "CLOCK_MONOTONIC_COARSE");
    lua_rawseti(L, -2, ++i);
#endif

#ifdef CLOCK_MONOTONIC_RAW
    lua_pushstring(L, "CLOCK_MONOTONIC_RAW");
    lua_rawseti(L, -2, ++i);
#endif

#ifdef CLOCK_MONOTONIC_RAW_APPROX
    lua_pushstring(L, "CLOCK_MONOTONIC_RAW_APPROX");
    lua_rawseti(L, -2, ++i);
#endif

#ifdef CLOCK_BOOTTIME
    lua_pushstring(L, "CLOCK_BOOTTIME");
    lua_rawseti(L, -2, ++i);
#endif

#ifdef CLOCK_UPTIME_RAW
    lua_pushstring(L, "CLOCK_UPTIME_RAW");
    lua_rawseti(L, -2, ++i);
#endif

#ifdef CLOCK_UPTIME_RAW_APPROX
    lua_pushstring(L, "CLOCK_UPTIME_RAW_APPROX");
    lua_rawseti(L, -2, ++i);
#endif

    return i;
  }
}
