// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_STOPWATCH_HPP
#define BRIGID_STOPWATCH_HPP

#include <lua.hpp>

#include <stdint.h>

namespace brigid {
  class stopwatch {
  public:
    virtual ~stopwatch() = 0;
    virtual const char* get_name() const = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual int64_t get_elapsed() const = 0;
  };

  stopwatch* new_stopwatch(lua_State*);
  stopwatch* new_stopwatch(lua_State*, const char*);
  int get_stopwatch_names(lua_State*, int);
}

#endif
