// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "stack_guard.hpp"

namespace brigid {
  stack_guard::stack_guard(lua_State* L)
    : state_(L),
      top_(lua_gettop(L)) {}

  stack_guard::~stack_guard() {
    lua_settop(state_, top_);
  }
}
