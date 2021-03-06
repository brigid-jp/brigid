// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_VIEW_HPP
#define BRIGID_VIEW_HPP

#include <brigid/noncopyable.hpp>

#include <lua.hpp>

#include <stddef.h>

namespace brigid {
  class view_t {
  public:
    view_t(const char*, size_t);
    void close();
    bool closed() const;
    const char* data() const;
    size_t size() const;
  private:
    const char* data_;
    size_t size_;
  };

  view_t* new_view(lua_State*, const char*, size_t);
  view_t* check_view(lua_State*, int);
  view_t* test_view(lua_State*, int);
}

#endif
