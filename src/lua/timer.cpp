// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/error.hpp>
#include <brigid/noncopyable.hpp>
#include "common.hpp"

#include <lua.hpp>

#include <time.h>
#include <system_error>

namespace brigid {
  namespace {
    static const clockid_t clock = CLOCK_MONOTONIC;

    class timer : private noncopyable {
    public:
      timer()
        : start_() {}

      void start() {
        if (clock_gettime(clock, &start_) == -1) {
          int code = errno;
          throw BRIGID_RUNTIME_ERROR(std::generic_category().message(code), make_error_code("error number", code));
        }
      }

      int64_t elapsed() const {
        struct timespec stop = {};
        if (clock_gettime(clock, &stop) == -1) {
          int code = errno;
          throw BRIGID_RUNTIME_ERROR(std::generic_category().message(code), make_error_code("error number", code));
        }
        int64_t sec = stop.tv_sec - start_.tv_sec;
        int32_t nsec = stop.tv_nsec - start_.tv_nsec;
        if (nsec < 0) {
          --sec;
          nsec += 1000000000;
        }
        return sec * 1000000000 + nsec;
      }

    private:
      struct timespec start_;
    };
  }

  void initialize_timer(lua_State* L) {

  }
}
