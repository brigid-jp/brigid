// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "util_lua.hpp"
#include "view.hpp"

#include <lua.hpp>

namespace brigid {
  using namespace lua;

  view_t::view_t(const char* data, size_t size)
    : data_(data),
      size_(size) {}

  void view_t::invalidate() {
    data_ = nullptr;
    size_ = 0;
  }

  const char* view_t::data() const {
    return data_;
  }

  size_t view_t::size() const {
    return size_;
  }

  view_invalidator::view_invalidator(view_t* view)
    : view_(view) {}

  view_invalidator::~view_invalidator() {
    view_->invalidate();
  }

  namespace {
    void impl_tostring(lua_State* L) {
      view_t* self = check_view(L, 1);
      push(L, self->data(), self->size());
    }
  }

  view_t* new_view(lua_State* L, const char* data, size_t size) {
    return new_userdata<view_t>(L, "brigid.view", data, size);
  }

  view_t* check_view(lua_State* L, int arg) {
    view_t* self = check_udata<view_t>(L, arg, "brigid.view");
    if (!self->data()) {
      luaL_error(L, "attempt to use an invalidated view");
    }
    return self;
  }

  void initialize_view(lua_State* L) {
    lua_newtable(L);
    {
      top_saver saver(L);
      luaL_newmetatable(L, "brigid.view");
      lua_pushvalue(L, -2);
      set_field(L, -2, "__index");
      set_field(L, -1, "__tostring", impl_tostring);
    }
    set_field(L, -2, "view");
  }
}
