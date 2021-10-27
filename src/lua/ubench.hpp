// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_UBENCH_HPP
#define BRIGID_UBENCH_HPP

#include <lua.hpp>

#include <stdint.h>
#include <iosfwd>

namespace brigid {
  namespace ubench {
    class stopwatch {
    public:
      virtual ~stopwatch() = 0;
      virtual void start() = 0;
      virtual void stop() = 0;
      virtual int64_t get_elapsed() const = 0;
    };

    void check_platform(std::ostream&);
    stopwatch* new_stopwatch_platform(lua_State*, const char*);
  }
}

#endif
