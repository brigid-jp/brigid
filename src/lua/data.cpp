// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "data.hpp"
#include "view.hpp"

#include <lua.hpp>

namespace brigid {
  data_t::data_t(const char* data, size_t size)
    : data_(data),
      size_(size) {}

  const char* data_t::data() const {
    return data_;
  }

  size_t data_t::size() const {
    return size_;
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
