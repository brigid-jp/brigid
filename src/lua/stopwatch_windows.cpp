// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/noncopyable.hpp>
#include "common.hpp"
#include "stopwatch.hpp"

#define NOMINMAX
#include <windows.h>

#include <stdint.h>
#include <string.h>

namespace brigid {
  namespace {
    // https://docs.microsoft.com/en-us/windows/win32/api/profileapi/nf-profileapi-queryperformancefrequency
    // On systems that run Windows XP or later, the function will always succeed and will thus never return zero.
    int64_t query_performance_frequency() {
      LARGE_INTEGER frequency = {};
      QueryPerformanceFrequency(&frequency);
      return frequency.QuadPart;
    }

    static const int64_t frequency = query_performance_frequency();

    class stopwatch_windows_impl : public stopwatch {
    public:
      stopwatch_windows_impl()
        : started(),
          stopped() {}

      virtual void start() {
        // https://docs.microsoft.com/en-us/windows/win32/api/profileapi/nf-profileapi-queryperformancecounter
        // On systems that run Windows XP or later, the function will always succeed and will thus never return zero.
        QueryPerformanceCounter(&started);
      }

      virtual void stop() {
        // https://docs.microsoft.com/en-us/windows/win32/api/profileapi/nf-profileapi-queryperformancecounter
        // On systems that run Windows XP or later, the function will always succeed and will thus never return zero.
        QueryPerformanceCounter(&stopped);
      }

      virtual const char* get_name() const {
        return "QueryPerformanceCounter";
      }

    protected:
      LARGE_INTEGER started;
      LARGE_INTEGER stopped;
    };

    class stopwatch_windows_10mhz : public stopwatch_windows_impl, private noncopyable {
    public:
      virtual int64_t get_elapsed() const {
        return (stopped.QuadPart - started.QuadPart) * 100;
      }
    };

    class stopwatch_windows : public stopwatch_windows_impl, private noncopyable {
    public:
      virtual int64_t get_elapsed() const {
        int64_t d = stopped.QuadPart - started.QuadPart;
        return d / frequency * 1000000000 + d % frequency * 1000000000 / frequency;
      }
    };
  }

  stopwatch* new_stopwatch(lua_State* L) {
    if (frequency == 10000000) {
      return new_userdata<stopwatch_windows_10mhz>(L, "brigid.stopwatch");
    } else {
      return new_userdata<stopwatch_windows>(L, "brigid.stopwatch");
    }
  }

  stopwatch* new_stopwatch(lua_State* L, const char* name) {
    if (strcmp(name, "QueryPerformanceCounter") == 0) {
      if (frequency == 10000000) {
        return new_userdata<stopwatch_windows_10mhz>(L, "brigid.stopwatch");
      } else {
        return new_userdata<stopwatch_windows>(L, "brigid.stopwatch");
      }
    }
    return nullptr;
  }

  int get_stopwatch_names(lua_State* L, int i) {
    lua_pushstring(L, "QueryPerformanceCounter");
    lua_rawseti(L, -2, ++i);
    return i;
  }
}
