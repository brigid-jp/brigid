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

  void view_t::close() {
    data_ = nullptr;
    size_ = 0;
  }

  bool view_t::closed() const {
    return !data_;
  }

  const char* view_t::data() const {
    return data_;
  }

  size_t view_t::size() const {
    return size_;
  }

  view_guard::view_guard(view_t* view)
    : view_(view) {}

  view_guard::~view_guard() {
    view_->close();
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
    if (self->closed()) {
      luaL_error(L, "attempt to use a closed brigid.view");
    }
    return self;
  }

  view_t* test_view(lua_State* L, int index) {
    if (view_t* self = test_udata<view_t>(L, index, "brigid.view")) {
      if (!self->closed()) {
        return self;
      }
    }
    return nullptr;
  }

  void initialize_view(lua_State* L) {
    lua_newtable(L);
    {
      stack_guard guard(L);
      luaL_newmetatable(L, "brigid.view");
      lua_pushvalue(L, -2);
      set_field(L, -2, "__index");
      set_field(L, -1, "__tostring", impl_tostring);
    }
    set_field(L, -2, "view");
  }
}
