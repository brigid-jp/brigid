// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_UTIL_LUA_HPP
#define BRIGID_UTIL_LUA_HPP

#include <lua.hpp>

#include <stddef.h>
#include <utility>

namespace brigid {
  namespace lua {
    using cxx_function_t = void (*)(lua_State*);

    int abs_index(lua_State*, int);
    void set_metatable(lua_State*, const char*);

    template <class T, class... T_args>
    inline T* new_userdata(lua_State* L, const char* name, T_args... args) {
      T* data = static_cast<T*>(lua_newuserdata(L, sizeof(T)));
      new(data) T(std::forward<T_args>(args)...);
      set_metatable(L, name);
      return data;
    }

    template <class T>
    inline T* check_udata(lua_State* L, int arg, const char* name) {
      return static_cast<T*>(luaL_checkudata(L, arg, name));
    }

    void push(lua_State*, const char*);
    void push(lua_State*, const char*, size_t);
    void push(lua_State*, cxx_function_t);

    template <class T, class... T_args>
    void set_field(lua_State* L, int index, T key, T_args... args) {
      index = abs_index(L, index);
      push(L, std::forward<T>(key));
      push(L, std::forward<T_args>(args)...);
      lua_settable(L, index);
    }
  }
}

#endif
