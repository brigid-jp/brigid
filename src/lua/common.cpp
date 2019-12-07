// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "common.hpp"
#include <exception>

namespace brigid {
  namespace {
    int closure(lua_State* L) {
      int top = lua_gettop(L);
      try {
        luax_function_t function = reinterpret_cast<luax_function_t>(lua_touserdata(L, lua_upvalueindex(1)));
        function(L);
        return lua_gettop(L) - top;
      } catch (const std::exception& e) {
        lua_settop(L, top);
        return luaL_error(L, "caught exception %s", e.what());
      }
      lua_settop(L, top);
      return 0;
    }
  }

  luax_data_reference::luax_data_reference(const char* data, size_t size)
    : data_(data),
      size_(size) {}

  const char* luax_data_reference::data() const {
    return data_;
  }

  size_t luax_data_reference::size() const {
    return size_;
  }

  std::string luax_data_reference::to_str() const {
    return std::string(data_, size_);
  }

  int luax_abs_index(lua_State* L, int index) {
#if LUA_VERSION_NUM >= 502
    return lua_absindex(L, index);
#else
    if (index < 0) {
      int top = lua_gettop(L);
      if (top >= -index) {
        return top + index + 1;
      }
    }
    return index;
#endif
  }

  luax_data_reference luax_check_data(lua_State* L, int arg) {
    size_t size = 0;
    const char* data = luaL_checklstring(L, arg, &size);
    return luax_data_reference(data, size);
  }

  void luax_push(lua_State* L, const char* data) {
    lua_pushstring(L, data);
  }

  void luax_push(lua_State* L, const char* data, size_t size) {
    lua_pushlstring(L, data, size);
  }

  void luax_push(lua_State* L, luax_function_t value) {
    lua_pushlightuserdata(L, reinterpret_cast<void*>(value));
    lua_pushcclosure(L, closure, 1);
  }

  void luax_set_field(lua_State* L, int index, const char* key, luax_function_t value) {
    index = luax_abs_index(L, index);
    luax_push(L, value);
    lua_setfield(L, index, key);
  }
}
