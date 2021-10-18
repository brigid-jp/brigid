// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "thread_reference.hpp"

#include <lua.hpp>

namespace brigid {
  thread_reference::thread_reference()
    : state_(),
      state_ref_(LUA_NOREF) {}

  thread_reference::thread_reference(lua_State* L)
    : state_(),
      state_ref_(LUA_NOREF) {
    state_ = lua_newthread(L);
    state_ref_ = luaL_ref(L, LUA_REGISTRYINDEX);
  }

  thread_reference::thread_reference(thread_reference&& that)
    : state_(that.state_),
      state_ref_(that.state_ref_) {
    that.reset();
  }

  thread_reference::~thread_reference() {
    unref();
  }

  thread_reference& thread_reference::operator=(thread_reference&& that) {
    if (this != &that) {
      unref();
      state_ = that.state_;
      state_ref_ = that.state_ref_;
      that.reset();
    }
    return *this;
  }

  lua_State* thread_reference::state() const {
    return state_;
  }

  void thread_reference::unref() {
    if (lua_State* L = state_) {
      luaL_unref(L, LUA_REGISTRYINDEX, state_ref_);
      reset();
    }
  }

  void thread_reference::reset() {
    state_ = nullptr;
    state_ref_ = LUA_NOREF;
  }
}
