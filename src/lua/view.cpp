// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/error.hpp>
#include "common.hpp"
#include "view.hpp"

#include <lua.hpp>

namespace brigid {
  namespace {
    void impl_get_string(lua_State* L) {
      view_t* self = check_view(L, 1);
      push(L, self->data(), self->size());
    }

    void impl_get_pointer(lua_State* L) {
      view_t* self = check_view(L, 1);
      get_field(L, LUA_REGISTRYINDEX, "brigid.common.decode_pointer");
      push(L, encode_pointer(self->data()));
      if (lua_pcall(L, 1, 1, 0) != 0) {
        throw BRIGID_LOGIC_ERROR(lua_tostring(L, -1));
      }
    }

    void impl_get_size(lua_State* L) {
      view_t* self = check_view(L, 1);
      push(L, self->size());
    }
  }

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
      luaL_newmetatable(L, "brigid.view");
      lua_pushvalue(L, -2);
      set_field(L, -2, "__index");
      lua_pop(L, 1);

      set_field(L, -1, "get_string", impl_get_string);
      set_field(L, -1, "get_pointer", impl_get_pointer);
      set_field(L, -1, "get_size", impl_get_size);
    }
    set_field(L, -2, "view");
  }
}
