// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/error.hpp>
#include <brigid/noncopyable.hpp>
#include "common.hpp"
#include "ubench.hpp"

#include <stdint.h>
#include <string.h>
#include <time.h>
#include <iostream>

namespace brigid {
  namespace ubench {
    namespace {
      static const char* names[] = {
        "CLOCK_REALTIME",             // [0]
        "CLOCK_REALTIME_COARSE",      // [1] linux
        "CLOCK_MONOTONIC",            // [2]
        "CLOCK_MONOTONIC_COARSE",     // [3] linux
        "CLOCK_MONOTONIC_RAW",        // [4] linux or apple
        "CLOCK_MONOTONIC_RAW_APPROX", // [5] linux
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
          int64_t u = stopped_.tv_sec - started_.tv_sec;
          int64_t v = stopped_.tv_nsec - started_.tv_nsec;
          u *= 1000000000;
          u += v;
          return u;
        }

        virtual const char* get_name() const {
          return names[T_name];
        }

        virtual double get_resolution() const {
          struct timespec resolution = {};
          if (clock_getres(T_clock, &resolution) == -1) {
            throw BRIGID_SYSTEM_ERROR();
          }
          int64_t u = resolution.tv_sec;
          int64_t v = resolution.tv_nsec;
          u *= 1000000000;
          u += v;
          return static_cast<double>(u);
        }

      private:
        struct timespec started_;
        struct timespec stopped_;
      };

      template <clockid_t T_clock, int T_name>
      stopwatch* new_stopwatch_unix(lua_State* L) {
        return new_userdata<stopwatch_unix<T_clock, T_name> >(L, "brigid.ubench.stopwatch");
      }

      void check_clock(std::ostream& out, const char* name, clockid_t clock) {
        struct timespec resolution = {};
        if (clock_getres(clock, &resolution) == -1) {
          throw BRIGID_SYSTEM_ERROR();
        }

        int64_t count;
        int64_t duration;
        struct timespec t;
        struct timespec u;

        if (clock_gettime(clock, &t) == -1) {
          throw BRIGID_SYSTEM_ERROR();
        }
        for (count = 1; ; ++count) {
          if (clock_gettime(clock, &u) == -1) {
            throw BRIGID_SYSTEM_ERROR();
          }
          duration = (u.tv_sec - t.tv_sec) * 1000000000 + u.tv_nsec - t.tv_nsec;
          if (duration > 0) {
            break;
          }
        }
        t = u;
        for (count = 1; ; ++count) {
          if (clock_gettime(clock, &u) == -1) {
            throw BRIGID_SYSTEM_ERROR();
          }
          duration = (u.tv_sec - t.tv_sec) * 1000000000 + u.tv_nsec - t.tv_nsec;
          if (duration > 0) {
            break;
          }
        }

        out
          << "check_clock: " << name << "\n"
          << "  resolution: " << resolution.tv_sec * 1000000000 + resolution.tv_nsec << "\n"
          << "  count: " << count << "\n"
          << "  duration: " << duration << "\n"
          << "  t: " << t.tv_sec * 1000000000 + t.tv_nsec << "\n"
          << "  u: " << u.tv_sec * 1000000000 + u.tv_nsec << "\n";
      }
    }

    void check_platform(std::ostream& out) {
      check_clock(out, "CLOCK_REALTIME", CLOCK_REALTIME);
#ifdef CLOCK_REALTIME_COARSE
      check_clock(out, "CLOCK_REALTIME_COARSE", CLOCK_REALTIME_COARSE);
#endif

      check_clock(out, "CLOCK_MONOTONIC", CLOCK_MONOTONIC);
#ifdef CLOCK_MONOTONIC_COARSE
      check_clock(out, "CLOCK_MONOTONIC_COARSE", CLOCK_MONOTONIC_COARSE);
#endif
#ifdef CLOCK_MONOTONIC_RAW
      check_clock(out, "CLOCK_MONOTONIC_RAW", CLOCK_MONOTONIC_RAW);
#endif
#ifdef CLOCK_MONOTONIC_RAW_APPROX
      check_clock(out, "CLOCK_MONOTONIC_RAW_APPROX", CLOCK_MONOTONIC_RAW_APPROX);
#endif

#ifdef CLOCK_BOOTTIME
      check_clock(out, "CLOCK_BOOTTIME", CLOCK_BOOTTIME);
#endif

#ifdef CLOCK_UPTIME_RAW
      check_clock(out, "CLOCK_UPTIME_RAW", CLOCK_UPTIME_RAW);
#endif
#ifdef CLOCK_UPTIME_RAW_APPROX
      check_clock(out, "CLOCK_UPTIME_RAW_APPROX", CLOCK_UPTIME_RAW_APPROX);
#endif
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
}
