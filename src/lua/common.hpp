// Copyright (c) 2019-2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_COMMON_HPP
#define BRIGID_COMMON_HPP

#include <brigid/type_traits.hpp>

#include <lua.hpp>

#include <cstddef>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>

namespace brigid {
  static const int check_validate_none = 0;
  static const int check_validate_not_closed = 1;
  static const int check_validate_not_running = 2;
  static const int check_validate_all = 3;

  namespace detail {
    void push_pointer(lua_State*, const void*);
    void* to_pointer(lua_State*, int);
  }

  int abs_index(lua_State*, int);
  int get_field(lua_State*, int, const char*);
  int new_metatable(lua_State*, const char*);
  void set_metatable(lua_State*, const char*);
  bool is_false(lua_State*, int);

  template <class T, enable_if_t<(std::is_integral<T>::value && std::is_signed<T>::value && sizeof(T) <= sizeof(lua_Integer)), std::nullptr_t> = nullptr>
  inline void push_integer(lua_State* L, T source) {
    lua_pushinteger(L, source);
  }

  template <class T, enable_if_t<(std::is_integral<T>::value && std::is_signed<T>::value && sizeof(T) > sizeof(lua_Integer)), std::nullptr_t> = nullptr>
  inline void push_integer(lua_State* L, T source) {
    static const T max = std::numeric_limits<lua_Integer>::max();
    static const T min = std::numeric_limits<lua_Integer>::min();
    if (min <= source && source <= max) {
      lua_pushinteger(L, static_cast<lua_Integer>(source));
    } else {
      lua_pushnumber(L, static_cast<lua_Number>(source));
    }
  }

  template <class T, enable_if_t<(std::is_integral<T>::value && std::is_unsigned<T>::value && sizeof(T) < sizeof(lua_Integer)), std::nullptr_t> = nullptr>
  inline void push_integer(lua_State* L, T source) {
    lua_pushinteger(L, source);
  }

  template <class T, enable_if_t<(std::is_integral<T>::value && std::is_unsigned<T>::value && sizeof(T) >= sizeof(lua_Integer)), std::nullptr_t> = nullptr>
  inline void push_integer(lua_State* L, T source) {
    static const T max = std::numeric_limits<lua_Integer>::max();
    if (source <= max) {
      lua_pushinteger(L, static_cast<lua_Integer>(source));
    } else {
      lua_pushnumber(L, static_cast<lua_Number>(source));
    }
  }

  template <class T, enable_if_t<(std::is_integral<T>::value && std::is_signed<T>::value && sizeof(T) < sizeof(lua_Integer)), std::nullptr_t> = nullptr>
  inline T check_integer(lua_State* L, int arg) {
    static const lua_Integer max = std::numeric_limits<T>::max();
    static const lua_Integer min = std::numeric_limits<T>::min();
    lua_Integer result = luaL_checkinteger(L, arg);
    if (min <= result && result <= max) {
      return static_cast<T>(result);
    }
    return luaL_argerror(L, arg, "out of bounds");
  }

  template <class T, enable_if_t<(std::is_integral<T>::value && std::is_signed<T>::value && sizeof(T) >= sizeof(lua_Integer)), std::nullptr_t> = nullptr>
  inline T check_integer(lua_State* L, int arg) {
    return luaL_checkinteger(L, arg);
  }

  template <class T, enable_if_t<(std::is_integral<T>::value && std::is_unsigned<T>::value && sizeof(T) < sizeof(lua_Integer)), std::nullptr_t> = nullptr>
  inline T check_integer(lua_State* L, int arg) {
    static const lua_Integer max = std::numeric_limits<T>::max();
    lua_Integer result = luaL_checkinteger(L, arg);
    if (0 <= result && result <= max) {
      return static_cast<T>(result);
    }
    return luaL_argerror(L, arg, "out of bounds");
  }

  template <class T, enable_if_t<(std::is_integral<T>::value && std::is_unsigned<T>::value && sizeof(T) >= sizeof(lua_Integer)), std::nullptr_t> = nullptr>
  inline T check_integer(lua_State* L, int arg) {
    lua_Integer result = luaL_checkinteger(L, arg);
    if (0 <= result) {
      return result;
    }
    return luaL_argerror(L, arg, "out of bounds");
  }

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

  template <class T, enable_if_t<std::is_pointer<T>::value, std::nullptr_t> = nullptr>
  inline void push_pointer(lua_State* L, T source) {
    detail::push_pointer(L, reinterpret_cast<const void*>(source));
  }

  template <class T, enable_if_t<std::is_pointer<T>::value, std::nullptr_t> = nullptr>
  inline T to_pointer(lua_State* L, int index) {
    return reinterpret_cast<T>(detail::to_pointer(L, index));
  }
}

#endif
