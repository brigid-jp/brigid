// Copyright (c) 2019,2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "common.hpp"
#include "function.hpp"
#include "view.hpp"

#include <lua.hpp>

namespace brigid {
  namespace {
    view_t* check_view(lua_State* L, int arg) {
      view_t* self = check_udata<view_t>(L, arg, "brigid.view");
      if (self->closed()) {
        luaL_argerror(L, arg, "attempt to use a closed brigid.view");
      }
      return self;
    }

    void impl_get_pointer(lua_State* L) {
      view_t* self = check_view(L, 1);
      push_pointer(L, self->data());
    }

    void impl_get_size(lua_State* L) {
      view_t* self = check_view(L, 1);
      push_integer(L, self->size());
    }

    void impl_get_string(lua_State* L) {
      view_t* self = check_view(L, 1);
      lua_pushlstring(L, self->data(), self->size());
    }
  }

  abstract_data_t* to_abstract_data_view(lua_State* L, int arg) {
    return test_udata<view_t>(L, arg, "brigid.view");
  }

  view_t::view_t(const char* data, size_t size)
    : data_(data),
      size_(size) {}

  bool view_t::closed() const {
    return !data_;
  }

  const char* view_t::data() const {
    return data_;
  }

  size_t view_t::size() const {
    return size_;
  }

  void view_t::close() {
    data_ = nullptr;
    size_ = 0;
  }

  view_t* new_view(lua_State* L, const char* data, size_t size) {
    return new_userdata<view_t>(L, "brigid.view", data, size);
  }

  void initialize_view(lua_State* L) {
    lua_newtable(L);
    {
      new_metatable(L, "brigid.view");
      lua_pushvalue(L, -2);
      lua_setfield(L, -2, "__index");
      decltype(function<impl_get_size>())::set_field(L, -1, "__len");
      decltype(function<impl_get_string>())::set_field(L, -1, "__tostring");
      lua_pop(L, 1);

      decltype(function<impl_get_pointer>())::set_field(L, -1, "get_pointer");
      decltype(function<impl_get_size>())::set_field(L, -1, "get_size");
      decltype(function<impl_get_string>())::set_field(L, -1, "get_string");
    }
    lua_setfield(L, -2, "view");
  }
}
