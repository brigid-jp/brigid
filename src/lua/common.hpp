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
#include <functional>
#include <limits>
#include <new>
#include <string>
#include <type_traits>
#include <utility>

namespace brigid {
  using cxx_function_t = void (*)(lua_State*);
  using lua_unsigned_t = std::make_unsigned<lua_Integer>::type;

  static constexpr int check_validate_none = 0;
  static constexpr int check_validate_not_closed = 1;
  static constexpr int check_validate_not_running = 2;
  static constexpr int check_validate_all = 3;

  int abs_index(lua_State*, int);
  int get_table(lua_State*, int);
  void new_metatable(lua_State*, const char*);
  void set_metatable(lua_State*, const char*);
  bool is_false(lua_State*, int);

  void push(lua_State*, lua_Integer); // TODO よく考える
  void push(lua_State*, const char*);
  void push(lua_State*, const char*, size_t);
  void push(lua_State*, const std::string&);
  void push(lua_State*, cxx_function_t);

  template <class T>
  inline void push_integer(lua_State* L, T source, enable_if_t<(std::is_integral<T>::value && std::is_signed<T>::value && sizeof(T) <= sizeof(lua_Integer))>* = nullptr) {
    lua_pushinteger(L, source);
  }

  template <class T>
  inline void push_integer(lua_State* L, T source, enable_if_t<(std::is_integral<T>::value && std::is_signed<T>::value && sizeof(T) > sizeof(lua_Integer))>* = nullptr) {
    static constexpr T max = std::numeric_limits<lua_Integer>::max();
    static constexpr T min = std::numeric_limits<lua_Integer>::min();
    if (min <= source && source <= max) {
      lua_pushinteger(L, source);
    } else {
      lua_pushnumber(L, source);
    }
  }

  template <class T>
  inline void push_integer(lua_State* L, T source, enable_if_t<(std::is_integral<T>::value && std::is_unsigned<T>::value && sizeof(T) < sizeof(lua_Integer))>* = nullptr) {
    lua_pushinteger(L, source);
  }

  template <class T>
  inline void push_integer(lua_State* L, T source, enable_if_t<(std::is_integral<T>::value && std::is_unsigned<T>::value && sizeof(T) >= sizeof(lua_Integer))>* = nullptr) {
    static constexpr T max = std::numeric_limits<lua_Integer>::max();
    if (source <= max) {
      lua_pushinteger(L, source);
    } else {
      lua_pushnumber(L, source);
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

  template <class T>
  inline void set_field(lua_State* L, int index, T key) {
    index = abs_index(L, index);
    push(L, std::forward<T>(key));
    lua_pushvalue(L, -2);
    lua_settable(L, index);
    lua_pop(L, 1);
  }

  inline void set_field(lua_State* L, int index, const char* key, cxx_function_t value) {
    index = abs_index(L, index);
    push(L, value);
    lua_setfield(L, index, key);
  }

  inline void set_metafield(lua_State* L, int index, const char* key, cxx_function_t value) {
    index = abs_index(L, index);
    if (lua_getmetatable(L, index)) {
      set_field(L, -1, key, value);
      lua_pop(L, 1);
    } else {
      lua_newtable(L);
      set_field(L, -1, key, value);
      lua_setmetatable(L, index);
    }
  }

  template <class T>
  inline int get_field(lua_State* L, int index, T key) {
    index = abs_index(L, index);
    push(L, std::forward<T>(key));
    return get_table(L, index);
  }

  class stack_guard : private noncopyable {
  public:
    explicit stack_guard(lua_State*);
    ~stack_guard();
  private:
    lua_State* state_;
    int top_;
  };

  class reference : private noncopyable {
  public:
    reference();
    reference(lua_State*, int);
    reference(reference&&);
    ~reference();
    reference& operator=(reference&&);
    lua_State* state() const;
    int get_field(lua_State*) const;
  private:
    lua_State* state_;
    int state_ref_;
    int ref_;
    void unref();
    void reset();
  };

  class scope_exit : private noncopyable {
  public:
    explicit scope_exit(std::function<void ()>);
    ~scope_exit();
  private:
    std::function<void ()> function_;
  };
}

#endif
