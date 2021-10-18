// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_THREAD_REFERENCE_HPP
#define BRIGID_THREAD_REFERENCE_HPP

#include <brigid/noncopyable.hpp>

#include <lua.hpp>

namespace brigid {
  class thread_reference : private noncopyable {
  public:
    thread_reference();
    explicit thread_reference(lua_State*);
    thread_reference(thread_reference&&);
    ~thread_reference();
    thread_reference& operator=(thread_reference&&);
    lua_State* state() const;
  private:
    lua_State* state_;
    int state_ref_;
    void unref();
    void reset();
  };
}

#endif
