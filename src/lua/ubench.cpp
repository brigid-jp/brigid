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

#ifdef _MSC_VER
#define NOMINMAX
#include <windows.h>
#endif

namespace brigid {
  namespace ubench {
    namespace {
      template <class T>
      void check_chrono(std::ostream& out, const char* name) {
        using std::chrono::duration_cast;
        using std::chrono::nanoseconds;

        int64_t count = 0;
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

      std::string result = out.str();
      lua_pushlstring(L, result.data(), result.size());
    }
#else
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

    void impl_check_runtime(lua_State* L) {
      std::ostringstream out;
      ubench::check_platform(out);
      ubench::check_chrono(out);
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
