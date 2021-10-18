// Copyright (c) 2019-2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_COMMON_HPP
#define BRIGID_COMMON_HPP

#include <brigid/noncopyable.hpp>
#include <brigid/type_traits.hpp>

#include <lua.hpp>

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <limits>
#include <new>
#include <string>
#include <type_traits>
#include <utility>

namespace brigid {
  using cxx_function_t = void (*)(lua_State*);

  static constexpr int check_validate_none = 0;
  static constexpr int check_validate_not_closed = 1;
  static constexpr int check_validate_not_running = 2;
  static constexpr int check_validate_all = 3;

  int abs_index(lua_State*, int);
  void new_metatable(lua_State*, const char*);
  void set_metatable(lua_State*, const char*);
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

  void push_handle_impl(lua_State*, const void*);

  template <class T>
  inline void push_handle(lua_State* L, T source, enable_if_t<(std::is_pointer<T>::value && sizeof(T) == sizeof(const void*))>* = nullptr) {
    push_handle_impl(L, reinterpret_cast<const void*>(source));
  }

  void push_pointer_impl(lua_State*, const void*);

  template <class T>
  inline void push_pointer(lua_State* L, T source, enable_if_t<(std::is_pointer<T>::value && sizeof(T) == sizeof(const void*))>* = nullptr) {
    push_pointer_impl(L, reinterpret_cast<const void*>(source));
  }

  void* to_handle_impl(lua_State*, int);

  template <class T>
  inline T to_handle(lua_State* L, int index, enable_if_t<(std::is_pointer<T>::value && sizeof(T) == sizeof(void*))>* = nullptr) {
    return reinterpret_cast<T>(to_handle_impl(L, index));
  }

  template <class T>
  inline T check_integer(lua_State* L, int arg, T min, T max, enable_if_t<(std::is_integral<T>::value && std::is_unsigned<T>::value)>* = nullptr) {
    intmax_t v = luaL_checkinteger(L, arg);
    if (v < 0) {
      return luaL_argerror(L, arg, "out of bounds");
    }
    uintmax_t u = v;
    if (u < min || u > max) {
      return luaL_argerror(L, arg, "out of bounds");
    }
    return static_cast<T>(u);
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

  void set_field(lua_State*, int, const char*, cxx_function_t);
  void set_metafield(lua_State*, int, const char*, cxx_function_t);
  int get_field(lua_State*, int, const char*);

  class reference : private noncopyable {
  public:
    reference();
    explicit reference(lua_State*);
    reference(reference&&);
    ~reference();
    reference& operator=(reference&&);
    lua_State* state() const;
  private:
    lua_State* state_;
    int state_ref_;
    void unref();
    void reset();
  };
}

#endif
