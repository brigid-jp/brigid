// Copyright (c) 2019-2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_COMMON_HPP
#define BRIGID_COMMON_HPP

#include <brigid/type_traits.hpp>

#include <lua.hpp>

#include <limits>
#include <memory>
#include <type_traits>
#include <utility>

namespace brigid {
  using cxx_function_t = void (*)(lua_State*);

  static constexpr int check_validate_none = 0;
  static constexpr int check_validate_not_closed = 1;
  static constexpr int check_validate_not_running = 2;
  static constexpr int check_validate_all = 3;

  namespace detail {
    void push_handle(lua_State*, const void*);
    void push_pointer(lua_State*, const void*);
    void* to_handle(lua_State*, int);
    void set_metatable(lua_State*, const char*);
  }

  int abs_index(lua_State*, int);
  int get_field(lua_State*, int, const char*);
  int new_metatable(lua_State*, const char*);
  bool is_false(lua_State*, int);

  template <class T>
  inline void push_integer(lua_State* L, T source, enable_if_t<(std::is_integral<T>::value && std::is_signed<T>::value && sizeof(T) <= sizeof(lua_Integer))>* = nullptr) {
    lua_pushinteger(L, static_cast<lua_Integer>(source));
  }

  template <class T>
  inline void push_integer(lua_State* L, T source, enable_if_t<(std::is_integral<T>::value && std::is_signed<T>::value && sizeof(T) > sizeof(lua_Integer))>* = nullptr) {
    static constexpr T max = std::numeric_limits<lua_Integer>::max();
    static constexpr T min = std::numeric_limits<lua_Integer>::min();
    if (min <= source && source <= max) {
      lua_pushinteger(L, static_cast<lua_Integer>(source));
    } else {
      lua_pushnumber(L, static_cast<lua_Number>(source));
    }
  }

  template <class T>
  inline void push_integer(lua_State* L, T source, enable_if_t<(std::is_integral<T>::value && std::is_unsigned<T>::value && sizeof(T) < sizeof(lua_Integer))>* = nullptr) {
    lua_pushinteger(L, static_cast<lua_Integer>(source));
  }

  template <class T>
  inline void push_integer(lua_State* L, T source, enable_if_t<(std::is_integral<T>::value && std::is_unsigned<T>::value && sizeof(T) >= sizeof(lua_Integer))>* = nullptr) {
    static constexpr T max = std::numeric_limits<lua_Integer>::max();
    if (source <= max) {
      lua_pushinteger(L, static_cast<lua_Integer>(source));
    } else {
      lua_pushnumber(L, static_cast<lua_Number>(source));
    }
  }

  void set_field(lua_State*, int, const char*, cxx_function_t);
  void set_metafield(lua_State*, int, const char*, cxx_function_t);

  template <class T, class... T_args>
  inline T* new_userdata(lua_State* L, const char* name, T_args... args) {
    T* data = static_cast<T*>(lua_newuserdata(L, sizeof(T)));
    new(data) T(std::forward<T_args>(args)...);
    detail::set_metatable(L, name);
    return data;
  }

  template <class T>
  inline T* check_udata(lua_State* L, int arg, const char* name) {
    return static_cast<T*>(luaL_checkudata(L, arg, name));
  }

  template <class T>
  inline void push_handle(lua_State* L, T source, enable_if_t<(std::is_pointer<T>::value && sizeof(T) == sizeof(const void*))>* = nullptr) {
    detail::push_handle(L, reinterpret_cast<const void*>(source));
  }

  template <class T>
  inline void push_pointer(lua_State* L, T source, enable_if_t<(std::is_pointer<T>::value && sizeof(T) == sizeof(const void*))>* = nullptr) {
    detail::push_pointer(L, reinterpret_cast<const void*>(source));
  }

  template <class T>
  inline T to_handle(lua_State* L, int index, enable_if_t<(std::is_pointer<T>::value && sizeof(T) == sizeof(void*))>* = nullptr) {
    return reinterpret_cast<T>(detail::to_handle(L, index));
  }
}

#endif
