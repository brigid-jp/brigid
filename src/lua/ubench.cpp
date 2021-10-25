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
  namespace {
    template <class T>
    void check_chrono(std::ostream& out, const char* name) {
      using time_point = typename T::time_point;
      time_point t = T::now();
      time_point u = T::now();
      out
        << "chrono: " << name << "\n"
        << std::chrono::duration_cast<std::chrono::nanoseconds>(u - time_point()).count() << "\n"
        << std::chrono::duration_cast<std::chrono::nanoseconds>(t - time_point()).count() << "\n"
        << std::chrono::duration_cast<std::chrono::nanoseconds>(u - t).count() << "\n";
    }

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

      LARGE_INTEGER f = {};
      LARGE_INTEGER t = {};
      LARGE_INTEGER u = {};
      if (!QueryPerformanceFrequency(&f)) {
        throw BRIGID_RUNTIME_ERROR("cannot QueryPerformanceFrequency");
      }
      if (!QueryPerformanceCounter(&t)) {
        throw BRIGID_RUNTIME_ERROR("cannot QueryPerformanceCounter");
      }
      if (!QueryPerformanceCounter(&u)) {
        throw BRIGID_RUNTIME_ERROR("cannot QueryPerformanceCounter");
      }
      int64_t d = u.QuadPart - t.QuadPart;

      out
        << "QueryPerformanceFrequency: " << f.QuadPart << "\n"
        << "QueryPerformanceCounter\n"
        << t.QuadPart << "\n"
        << u.QuadPart << "\n"
        << d << "\n";

      check_chrono<std::chrono::system_clock>(out, "std::chrono::system_clock");
      check_chrono<std::chrono::steady_clock>(out, "std::chrono::steady_clock");
      check_chrono<std::chrono::high_resolution_clock>(out, "std::chrono::high_resolution_clock");

      std::string result = out.str();
      lua_pushlstring(L, result.data(), result.size());
    }
#else
    static const clockid_t clock = CLOCK_MONOTONIC;

    int64_t sub(const struct timespec& t, const struct timespec& u) {
      int64_t s = t.tv_sec - u.tv_sec;
      int32_t n = t.tv_nsec - u.tv_nsec;
      if (n < 0) {
        --s;
        n += 1000000000;
      }
      return s * 1000000000 + n;
    }

    void check_clock(std::ostream& out, const char* name, clockid_t clock) {
      struct timespec r = {};
      struct timespec t = {};
      struct timespec u = {};

      if (clock_getres(clock, &r) == -1) {
        throw BRIGID_SYSTEM_ERROR();
      }
      if (clock_gettime(clock, &t) == -1) {
        throw BRIGID_SYSTEM_ERROR();
      }
      if (clock_gettime(clock, &u) == -1) {
        throw BRIGID_SYSTEM_ERROR();
      }
      int64_t d = sub(u, t);

      out
        << "clock_getres: " << name << "\n"
        << r.tv_sec << "." << std::setw(9) << r.tv_nsec << "\n"
        << "clock_gettime: " << name << "\n"
        << t.tv_sec << "." << std::setw(9) << t.tv_nsec << "\n"
        << u.tv_sec << "." << std::setw(9) << u.tv_nsec << "\n"
        << d << "\n";
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

      check_chrono<std::chrono::system_clock>(out, "std::chrono::system_clock");
      check_chrono<std::chrono::steady_clock>(out, "std::chrono::steady_clock");
      check_chrono<std::chrono::high_resolution_clock>(out, "std::chrono::high_resolution_clock");

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
