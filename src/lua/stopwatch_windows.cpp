// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/error.hpp>
#include <brigid/noncopyable.hpp>
#include "common.hpp"
#include "stopwatch.hpp"

#define NOMINMAX
#include <windows.h>

#include <stdint.h>
#include <string.h>

namespace brigid {
  namespace {
    int64_t query_performance_frequency() {
      LARGE_INTEGER frequency = {};
      if (!QueryPerformanceFrequency(&frequency)) {
        throw BRIGID_RUNTIME_ERROR("cannot QueryPerformanceFrequency");
      }
      return frequency.QuadPart;
    }

    static const int64_t frequency = query_performance_frequency();

    class stopwatch_windows_10mhz : public stopwatch, private noncopyable {
    public:
      stopwatch_windows_10mhz()
        : started_(),
          stopped_() {}

      virtual void start() {
        if (!QueryPerformanceCounter(&started_)) {
          throw BRIGID_RUNTIME_ERROR("cannot QueryPerformanceCounter");
        }
      }

      virtual void stop() {
        if (!QueryPerformanceCounter(&stopped_)) {
          throw BRIGID_RUNTIME_ERROR("cannot QueryPerformanceCounter");
        }
      }

      virtual int64_t get_elapsed() const {
        return (stopped_.QuadPart - started_.QuadPart) * 100;
      }

      virtual const char* get_name() const {
        return "QueryPerformanceCounter";
      }

      virtual double get_resolution() const {
        return 100;
      }

    private:
      LARGE_INTEGER started_;
      LARGE_INTEGER stopped_;
    };

    class stopwatch_windows : public stopwatch, private noncopyable {
    public:
      explicit stopwatch_windows()
        : started_(),
          stopped_() {}

      virtual void start() {
        if (!QueryPerformanceCounter(&started_)) {
          throw BRIGID_RUNTIME_ERROR("cannot QueryPerformanceCounter");
        }
      }

      virtual void stop() {
        if (!QueryPerformanceCounter(&stopped_)) {
          throw BRIGID_RUNTIME_ERROR("cannot QueryPerformanceCounter");
        }
      }

      virtual int64_t get_elapsed() const {
        int64_t u = stopped_.QuadPart - started_.QuadPart;
        int64_t v = u % frequency;
        u /= frequency;
        u *= 1000000000;
        v *= 1000000000;
        v /= frequency;
        u += v;
        return u;
      }

      virtual const char* get_name() const {
        return "QueryPerformanceCounter";
      }

      virtual double get_resolution() const {
        double u = 1000000000;
        u /= frequency;
        return u;
      }

    private:
      LARGE_INTEGER started_;
      LARGE_INTEGER stopped_;
    };
  }

  stopwatch* new_stopwatch(lua_State* L, const char* name) {
    if (!name || strcasecmp(name, "QueryPerformanceCounter")) {
      if (frequency == 10000000) {
        return new_userdata<stopwatch_windows_10mhz(L, "brigid.stopwatch");
      } else {
        return new_userdata<stopwatch_windows(L, "brigid.stopwatch");
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
