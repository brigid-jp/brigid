// Copyright (c) 2021,2024 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_STACK_GUARD_HPP
#define BRIGID_STACK_GUARD_HPP

#include "noncopyable.hpp"

#include <lua.hpp>

namespace brigid {
  class stack_guard : private noncopyable {
  public:
    explicit stack_guard(lua_State*);
    ~stack_guard();
    int top() const;
  private:
    lua_State* state_;
    int top_;
  };
}

#endif
