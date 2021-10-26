// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/error.hpp>
#include <brigid/noncopyable.hpp>
#include "common.hpp"

#include <lua.hpp>

#include <stdint.h>
#include <time.h>
#include <chrono>
#include <iomanip>
#include <sstream>

#ifdef _MSC_VER
#define NOMINMAX
#include <windows.h>
#endif

namespace brigid {
  namespace ubench {
    template <class T>
    void check_chrono(std::ostream& out, const char* name) {
      using duration = typename T::duration;
      using time_point = typename T::time_point;
      using std::chrono::duration_cast;
      using std::chrono::nanoseconds;

      int64_t i = 0;
      duration d = {};
      time_point u = {};

      time_point t = T::now();
      for (i = 1; ; ++i) {
        u = T::now();
        d = u - t;
        if (d > duration::zero()) {
          break;
        }
      }

      out
        << "check_chrono: " << name << "\n"
        << "count: " << i << "\n"
        << "duration: " << duration_cast<nanoseconds>(d).count() << "\n"
        << "t: " << duration_cast<nanoseconds>(t.time_since_epoch()).count() << "\n"
        << "u: " << duration_cast<nanoseconds>(u.time_since_epoch()).count() << "\n";
    }
  }

  namespace {
#ifdef _MSC_VER
    class stopwatch : private noncopyable {
    public:
      void start() {}
      void stop() {}
      int64_t get_elapsed() const {
        return 1;
      }

    private:
    };

    void impl_check_runtime(lua_State* L) {
      std::ostringstream out;
      out << std::setfill('0');

      int64_t i = 0;
      int64_t d = 0;
      LARGE_INTEGER f = {};
      LARGE_INTEGER t = {};
      LARGE_INTEGER u = {};

      if (!QueryPerformanceFrequency(&f)) {
        throw BRIGID_RUNTIME_ERROR("cannot QueryPerformanceFrequency");
      }
      if (!QueryPerformanceCounter(&t)) {
        throw BRIGID_RUNTIME_ERROR("cannot QueryPerformanceCounter");
      }
      for (i = 1; ; ++i) {
        if (!QueryPerformanceCounter(&u)) {
          throw BRIGID_RUNTIME_ERROR("cannot QueryPerformanceCounter");
        }
        d = u.QuadPart - t.QuadPart;
        if (d > 0) {
          break;
        }
      }

      if (f.QuadPart == 10000000) {
        out << "QueryPerformanceCounter: 10MHz\n";
      } else {
        out << "QueryPerformanceCounter: " << f.QuadPart  << "Hz\n";
      }

      out
        << "QueryPerformanceCounter\n"
        << "count: " << i << "\n"
        << "d: " << d << "\n"
        << "duration: " << (d * 1000000000 / f.QuadPart) << "\n"
        << "t: " << t.QuadPart << "\n"
        << "u: " << u.QuadPart << "\n";

      ubench::check_chrono<std::chrono::system_clock>(out, "std::chrono::system_clock");
      ubench::check_chrono<std::chrono::steady_clock>(out, "std::chrono::steady_clock");
      ubench::check_chrono<std::chrono::high_resolution_clock>(out, "std::chrono::high_resolution_clock");

      std::string result = out.str();
      lua_pushlstring(L, result.data(), result.size());
    }
#else
    static const clockid_t clock = CLOCK_MONOTONIC;

    void check_clock(std::ostream& out, const char* name, clockid_t clock) {
      int64_t i = 0;
      int64_t d = 0;
      struct timespec r = {};
      struct timespec t = {};
      struct timespec u = {};

      if (clock_getres(clock, &r) == -1) {
        throw BRIGID_SYSTEM_ERROR();
      }
      if (clock_gettime(clock, &t) == -1) {
        throw BRIGID_SYSTEM_ERROR();
      }
      for (i = 1; ; ++i) {
        if (clock_gettime(clock, &u) == -1) {
          throw BRIGID_SYSTEM_ERROR();
        }
        int64_t s = u.tv_sec - t.tv_sec;
        int32_t n = u.tv_nsec - t.tv_nsec;
        if (n < 0) {
          --s;
          n += 1000000000;
        }
        d = s * 1000000000 + n;
        if (d > 0) {
          break;
        }
      }

      out
        << "check_clock: " << name << "\n"
        << "resolution: " << (r.tv_sec * 1000000000 + r.tv_nsec ) << "\n"
        << "count: " << i << "\n"
        << "duration: " << d << "\n"
        << "t: " << (t.tv_sec * 1000000000 + t.tv_nsec) << "\n"
        << "u: " << (u.tv_sec * 1000000000 + u.tv_nsec) << "\n";
    }

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

    void impl_check_runtime(lua_State* L) {
      std::ostringstream out;
      out << std::setfill('0');

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

      ubench::check_chrono<std::chrono::system_clock>(out, "std::chrono::system_clock");
      ubench::check_chrono<std::chrono::steady_clock>(out, "std::chrono::steady_clock");
      ubench::check_chrono<std::chrono::high_resolution_clock>(out, "std::chrono::high_resolution_clock");

      std::string result = out.str();
      lua_pushlstring(L, result.data(), result.size());
    }
#endif

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
      set_field(L, -1, "check_runtime", impl_check_runtime);

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
