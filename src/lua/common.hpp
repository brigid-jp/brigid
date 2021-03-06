// Copyright (c) 2019 <dev@brigid.jp>
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

  static const int check_validate_none = 0;
  static const int check_validate_not_closed = 1;
  static const int check_validate_not_running = 2;
  static const int check_validate_all = 3;

  int abs_index(lua_State*, int);
  int get_table(lua_State*, int);
  void set_metatable(lua_State*, const char*);
  void* test_udata_impl(lua_State*, int, const char*);
  bool is_false(lua_State*, int);

  void push(lua_State*, lua_Integer);
  void push(lua_State*, const char*);
  void push(lua_State*, const char*, size_t);
  void push(lua_State*, const std::string&);
  void push(lua_State*, cxx_function_t);

  template <class T>
  inline std::string encode_pointer(T source, enable_if_t<std::is_pointer<T>::value>* = nullptr) {
    static const size_t size = sizeof(source);
    char buffer[size] = {};
    memmove(buffer, &source, size);
    return std::string(buffer, size);
  }

  template <class T>
  inline T decode_pointer(const char* data, size_t size, enable_if_t<std::is_pointer<T>::value>* = nullptr) {
    T result = nullptr;
    if (data && size == sizeof(T)) {
      memmove(&result, data, size);
    }
    return result;
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
  inline T* test_udata(lua_State* L, int index, const char* name) {
    return static_cast<T*>(test_udata_impl(L, index, name));
  }

  template <class T>
  inline void set_field(lua_State* L, int index, T key) {
    index = abs_index(L, index);
    push(L, std::forward<T>(key));
    lua_pushvalue(L, -2);
    lua_settable(L, index);
    lua_pop(L, 1);
  }

  template <class T, class... T_args>
  inline void set_field(lua_State* L, int index, T key, T_args... args) {
    index = abs_index(L, index);
    push(L, std::forward<T>(key));
    push(L, std::forward<T_args>(args)...);
    lua_settable(L, index);
  }

  template <class... T>
  inline void set_metafield(lua_State* L, int index, T... args) {
    index = abs_index(L, index);
    if (lua_getmetatable(L, index)) {
      set_field(L, -1, std::forward<T>(args)...);
      lua_pop(L, 1);
    } else {
      lua_newtable(L);
      set_field(L, -1, std::forward<T>(args)...);
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
}

#endif
