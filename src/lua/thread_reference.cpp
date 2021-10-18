// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "thread_reference.hpp"

#include <lua.hpp>

namespace brigid {
  thread_reference::thread_reference()
    : thread_(),
      ref_(LUA_NOREF) {}

  thread_reference::thread_reference(lua_State* L)
    : thread_(),
      ref_(LUA_NOREF) {
    thread_ = lua_newthread(L);
    ref_ = luaL_ref(L, LUA_REGISTRYINDEX);
  }

  thread_reference::thread_reference(thread_reference&& that)
    : thread_(that.thread_),
      ref_(that.ref_) {
    that.reset();
  }

  thread_reference::~thread_reference() {
    unref();
  }

  thread_reference& thread_reference::operator=(thread_reference&& that) {
    if (this != &that) {
      unref();
      thread_ = that.thread_;
      ref_ = that.ref_;
      that.reset();
    }
    return *this;
  }

  lua_State* thread_reference::get() const {
    return thread_;
  }

  thread_reference::operator bool() const {
    return thread_;
  }

  void thread_reference::unref() {
    if (lua_State* L = thread_) {
      luaL_unref(L, LUA_REGISTRYINDEX, ref_);
      reset();
    }
  }

  void thread_reference::reset() {
    thread_ = nullptr;
    ref_ = LUA_NOREF;
  }
}
