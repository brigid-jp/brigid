// Copyright (c) 2019-2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_COMMON_HPP
#define BRIGID_COMMON_HPP

#include <brigid/type_traits.hpp>

#include <lua.hpp>

#include <exception>
#include <limits>
#include <memory>
#include <stdexcept>
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
    void set_field(lua_State*, int, const char*, lua_CFunction);
    void set_metafield(lua_State*, int, const char*, lua_CFunction);
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
      lua_pushinteger(L, source);
    } else {
      lua_pushnumber(L, source);
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
      lua_pushinteger(L, source);
    } else {
      lua_pushnumber(L, source);
    }
  }

  template <class T>
  inline T check_integer(lua_State* L, int arg, enable_if_t<(std::is_integral<T>::value && std::is_signed<T>::value && sizeof(T) < sizeof(lua_Integer))>* = nullptr) {
    static const lua_Integer max = std::numeric_limits<T>::max();
    static const lua_Integer min = std::numeric_limits<T>::min();
    lua_Integer result = luaL_checkinteger(L, arg);
    if (min <= result && result <= max) {
      return result;
    }
    return luaL_argerror(L, arg, "out of bounds");
  }

  template <class T>
  inline T check_integer(lua_State* L, int arg, enable_if_t<(std::is_integral<T>::value && std::is_signed<T>::value && sizeof(T) >= sizeof(lua_Integer))>* = nullptr) {
    return luaL_checkinteger(L, arg);
  }

  template <class T>
  inline T check_integer(lua_State* L, int arg, enable_if_t<(std::is_integral<T>::value && std::is_unsigned<T>::value && sizeof(T) < sizeof(lua_Integer))>* = nullptr) {
    static const lua_Integer max = std::numeric_limits<T>::max();
    lua_Integer result = luaL_checkinteger(L, arg);
    if (0 <= result && result <= max) {
      return result;
    }
    return luaL_argerror(L, arg, "out of bounds");
  }

  template <class T>
  inline T check_integer(lua_State* L, int arg, enable_if_t<(std::is_integral<T>::value && std::is_unsigned<T>::value && sizeof(T) >= sizeof(lua_Integer))>* = nullptr) {
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

  template <class T>
  inline void push_pointer(lua_State* L, T source, enable_if_t<(std::is_pointer<T>::value && sizeof(T) == sizeof(const void*))>* = nullptr) {
    detail::push_pointer(L, reinterpret_cast<const void*>(source));
  }

  template <class T>
  inline T to_pointer(lua_State* L, int index, enable_if_t<(std::is_pointer<T>::value && sizeof(T) == sizeof(void*))>* = nullptr) {
    return reinterpret_cast<T>(detail::to_pointer(L, index));
  }

  template <class T, T (*)(lua_State*)>
  struct function_wrapper;

  template <int (*T)(lua_State*)>
  struct function_wrapper<int, T> {
    static int value(lua_State* L) {
      try {
        return T(L);
      } catch (const std::runtime_error& e) {
        lua_pushnil(L);
        lua_pushstring(L, e.what());
        return 2;
      } catch (const std::exception& e) {
        return luaL_error(L, "%s", e.what());
      }
    }

    static void set_field(lua_State* L, int index, const char* key) {
      detail::set_field(L, index, key, value);
    }

    static void set_metafield(lua_State* L, int index, const char* key) {
      detail::set_metafield(L, index, key, value);
    }
  };

  template <void (*T)(lua_State*)>
  struct function_wrapper<void, T> {
    static int value(lua_State* L) {
      try {
        int top = lua_gettop(L);
        T(L);
        int result = lua_gettop(L) - top;
        if (result > 0) {
          return result;
        } else {
          if (lua_toboolean(L, 1)) {
            lua_pushvalue(L, 1);
          } else {
            lua_pushboolean(L, true);
          }
          return 1;
        }
      } catch (const std::runtime_error& e) {
        lua_pushnil(L);
        lua_pushstring(L, e.what());
        return 2;
      } catch (const std::exception& e) {
        return luaL_error(L, "%s", e.what());
      }
    }

    static void set_field(lua_State* L, int index, const char* key) {
      detail::set_field(L, index, key, value);
    }

    static void set_metafield(lua_State* L, int index, const char* key) {
      detail::set_metafield(L, index, key, value);
    }
  };

  template <int (*T)(lua_State*)>
  function_wrapper<int, T> function();

  template <void (*T)(lua_State*)>
  function_wrapper<void , T> function();
}

#endif
