// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/error.hpp>
#include <brigid/noncopyable.hpp>
#include "common.hpp"
#include "ubench.hpp"

#include <stdint.h>
#include <time.h>
#include <iostream>

namespace brigid {
  namespace ubench {
    namespace {
      static const clockid_t default_clock =
#ifdef CLOCK_MONOTONIC_RAW
        CLOCK_MONOTONIC_RAW
#else
        CLOCK_MONOTONIC
#endif
      ;

      class stopwatch_impl : public stopwatch, private noncopyable {
      public:
        explicit stopwatch_impl(clockid_t clock)
          : clock_(clock),
            started_(),
            stopped_() {}

        virtual void start() {
          if (clock_gettime(clock_, &started_) == -1) {
            throw BRIGID_SYSTEM_ERROR();
          }
        }

        virtual void stop() {
          if (clock_gettime(clock_, &stopped_) == -1) {
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

      private:
        clockid_t clock_;
        struct timespec started_;
        struct timespec stopped_;
      };

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

    stopwatch* new_stopwatch_platform(lua_State* L, const char* name) {
      if (!name) {
        return new_userdata<stopwatch_impl>(L, "brigid.ubench.stopwatch", default_clock);
      }
      return nullptr;
    }
  }
}
