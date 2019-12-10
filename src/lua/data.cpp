// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "data.hpp"
#include "view.hpp"

#include <lua.hpp>

#include <stddef.h>
#include <string>

namespace brigid {
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
      if (view_t* view = test_view(L, index)) {
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
      view_t* view = check_view(L, arg);
      return data_t(view->data(), view->size());
    } else {
      size_t size = 0;
      const char* data = luaL_checklstring(L, arg, &size);
      return data_t(data, size);
    }
  }
}
