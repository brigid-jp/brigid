// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "stack_guard.hpp"

#include <lua.hpp>

namespace brigid {
  stack_guard::stack_guard(lua_State* L)
    : state_(L),
      top_(lua_gettop(L)) {}

  stack_guard::~stack_guard() {
    lua_settop(state_, top_);
  }

  int stack_guard::top() const {
    return top_;
  }
}
