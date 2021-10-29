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

static_assert(std::is_same<lua_Number, double>::value, "lua_Number is not double");

namespace brigid {
  using void_function_t = void (*)(lua_State*);

  static const int check_validate_none = 0;
  static const int check_validate_not_closed = 1;
  static const int check_validate_not_running = 2;
  static const int check_validate_all = 3;

  namespace detail {
    void push_handle(lua_State*, const void*);
    void push_pointer(lua_State*, const void*);
    void* to_handle(lua_State*, int);
  }

  int abs_index(lua_State*, int);
  int get_field(lua_State*, int, const char*);
  int new_metatable(lua_State*, const char*);
  void set_metatable(lua_State*, const char*);
  bool is_false(lua_State*, int);

  template <class T>
  inline void push_integer(lua_State* L, T source, enable_if_t<(std::is_integral<T>::value && std::is_signed<T>::value && sizeof(T) <= sizeof(lua_Integer))>* = nullptr) {
    lua_pushinteger(L, static_cast<lua_Integer>(source));
  }

  template <class T>
  inline void push_integer(lua_State* L, T source, enable_if_t<(std::is_integral<T>::value && std::is_signed<T>::value && sizeof(T) > sizeof(lua_Integer))>* = nullptr) {
    static const T max = std::numeric_limits<lua_Integer>::max();
    static const T min = std::numeric_limits<lua_Integer>::min();
    if (min <= source && source <= max) {
      lua_pushinteger(L, static_cast<lua_Integer>(source));
    } else {
      lua_pushnumber(L, static_cast<double>(source));
    }
  }

  template <class T>
  inline void push_integer(lua_State* L, T source, enable_if_t<(std::is_integral<T>::value && std::is_unsigned<T>::value && sizeof(T) < sizeof(lua_Integer))>* = nullptr) {
    lua_pushinteger(L, static_cast<lua_Integer>(source));
  }

  template <class T>
  inline void push_integer(lua_State* L, T source, enable_if_t<(std::is_integral<T>::value && std::is_unsigned<T>::value && sizeof(T) >= sizeof(lua_Integer))>* = nullptr) {
    static const T max = std::numeric_limits<lua_Integer>::max();
    if (source <= max) {
      lua_pushinteger(L, static_cast<lua_Integer>(source));
    } else {
      lua_pushnumber(L, static_cast<double>(source));
    }
  }

  template <class T>
  inline T check_integer(lua_State* L, int arg, enable_if_t<(std::is_integral<T>::value && std::is_signed<T>::value && sizeof(T) < sizeof(lua_Integer))>* = nullptr) {
    static const lua_Integer max = std::numeric_limits<T>::max();
    static const lua_Integer min = std::numeric_limits<T>::min();
    lua_Integer result = luaL_checkinteger(L, arg);
    if (min <= result && result <= max) {
      return static_cast<T>(result);
    }
    return luaL_argerror(L, arg, "out of bounds");
  }

  template <class T>
  inline T check_integer(lua_State* L, int arg, enable_if_t<(std::is_integral<T>::value && std::is_signed<T>::value && sizeof(T) >= sizeof(lua_Integer))>* = nullptr) {
    return static_cast<T>(luaL_checkinteger(L, arg));
  }

  template <class T>
  inline T check_integer(lua_State* L, int arg, enable_if_t<(std::is_integral<T>::value && std::is_unsigned<T>::value && sizeof(T) < sizeof(lua_Integer))>* = nullptr) {
    static const lua_Integer max = std::numeric_limits<T>::max();
    lua_Integer result = luaL_checkinteger(L, arg);
    if (0 <= result && result <= max) {
      return static_cast<T>(result);
    }
    return luaL_argerror(L, arg, "out of bounds");
  }

  template <class T>
  inline T check_integer(lua_State* L, int arg, enable_if_t<(std::is_integral<T>::value && std::is_unsigned<T>::value && sizeof(T) >= sizeof(lua_Integer))>* = nullptr) {
    lua_Integer result = luaL_checkinteger(L, arg);
    if (0 <= result) {
      return static_cast<T>(result);
    }
    return luaL_argerror(L, arg, "out of bounds");
  }

  void set_field(lua_State*, int, const char*, lua_CFunction);
  void set_field(lua_State*, int, const char*, void_function_t);
  void set_metafield(lua_State*, int, const char*, void_function_t);

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

  template <void (*T)(lua_State*)>
  struct void_function {
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
      index = abs_index(L, index);
      lua_pushcfunction(L, value);
      lua_setfield(L, index, key);
    }

    static void set_metafield(lua_State* L, int index, const char* key) {
      index = abs_index(L, index);
      if (lua_getmetatable(L, index)) {
        set_field(L, -1, key);
        lua_pop(L, 1);
      } else {
        lua_newtable(L);
        set_field(L, -1, key);
        lua_setmetatable(L, index);
      }
    }
  };

  template <int (*T)(lua_State*)>
  struct int_function {
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
      index = abs_index(L, index);
      lua_pushcfunction(L, value);
      lua_setfield(L, index, key);
    }
  };
}

#endif
