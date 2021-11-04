// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_THREAD_REFERENCE_HPP
#define BRIGID_THREAD_REFERENCE_HPP

#include "noncopyable.hpp"

#include <lua.hpp>

namespace brigid {
  class thread_reference : private noncopyable {
  public:
    thread_reference();
    explicit thread_reference(lua_State*);
    thread_reference(thread_reference&&);
    ~thread_reference();
    thread_reference& operator=(thread_reference&&);
    lua_State* get() const;
    explicit operator bool() const;
  private:
    lua_State* thread_;
    int ref_;
    void unref();
    void reset();
  };
}

#endif
