// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/error.hpp>
#include "util_lua.hpp"
#include "view.hpp"

#include <lua.hpp>

#include <iostream>

namespace brigid {
  using namespace util_lua;

  namespace {
    typedef struct {
      const void* (*f)(void*);
    } get_pointer_ffi_t;

    const void* get_pointer_ffi_f(void* self) {
      return static_cast<view_t*>(self)->data();
    }

    get_pointer_ffi_t get_pointer_ffi = { &get_pointer_ffi_f };

    void initialize_view_get_pointer_ffi(lua_State* L) {
      stack_guard guard(L);

      static const char code[] =
      #include "view.lua"
      ;

      if (luaL_loadstring(L, code) != 0) {
        throw BRIGID_ERROR(lua_tostring(L, -1));
      }
      push(L, encode_pointer(&get_pointer_ffi));
      if (lua_pcall(L, 1, 1, 0) != 0) {
        throw BRIGID_ERROR(lua_tostring(L, -1));
      }
      set_field(L, -2, "get_pointer_ffi");
    }

    void impl_tostring(lua_State* L) {
      view_t* self = check_view(L, 1);
      push(L, self->data(), self->size());
    }

    void impl_get_pointer(lua_State* L) {
      // ffi
      // useradata
      // string
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
      set_field(L, -1, "__tostring", impl_tostring);
      lua_pop(L, 1);

      initialize_view_get_pointer_ffi(L);
    }
    set_field(L, -2, "view");
  }
}
