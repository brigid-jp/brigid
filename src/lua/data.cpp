// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/error.hpp>
#include "common.hpp"
#include "data.hpp"
#include "view.hpp"

#include <lua.hpp>

#include <stddef.h>
#include <string>

namespace brigid {
  namespace {
    bool is_love2d_data(lua_State* L, int index, data_t& result) {
      stack_guard guard(L);
      index = abs_index(L, index);
      get_field(L, LUA_REGISTRYINDEX, "brigid.common.is_love2d_data");
      lua_pushvalue(L, index);
      if (lua_pcall(L, 1, 2, 0) != 0) {
        throw BRIGID_LOGIC_ERROR(lua_tostring(L, -1));
      }
      if (!lua_isnil(L, -2)) {
        size_t size = 0;
        const char* data = lua_tolstring(L, -2, &size);
        result = data_t(decode_pointer<const char*>(data, size), lua_tointeger(L, -1));
        return true;
      }
      return false;
    }
  }

  data_t::data_t()
    : data_(),
      size_() {}

  data_t::data_t(const char* data, size_t size)
    : data_(data),
      size_(size) {}

  const char* data_t::data() const {
    return data_;
  }

  size_t data_t::size() const {
    return size_;
  }

  std::string data_t::str() const {
    if (size_ == 0) {
      return std::string();
    } else {
      return std::string(data_, size_);
    }
  }

  data_t to_data(lua_State* L, int index) {
    if (lua_isuserdata(L, index)) {
      data_t result;
      if (is_love2d_data(L, index, result)) {
        return result;
      } else if (view_t* view = test_view(L, index)) {
        return data_t(view->data(), view->size());
      }
    } else {
      size_t size = 0;
      if (const char* data = lua_tolstring(L, index, &size)) {
        return data_t(data, size);
      }
    }
    return data_t();
  }

  data_t check_data(lua_State* L, int arg) {
    if (lua_isuserdata(L, arg)) {
      data_t result;
      if (is_love2d_data(L, arg, result)) {
        return result;
      } else {
        view_t* view = check_view(L, arg);
        return data_t(view->data(), view->size());
      }
    } else {
      size_t size = 0;
      const char* data = luaL_checklstring(L, arg, &size);
      return data_t(data, size);
    }
  }
}
