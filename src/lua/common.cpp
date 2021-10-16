// Copyright (c) 2019-2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/error.hpp>
#include "common.hpp"

#include <lua.hpp>

#include <stddef.h>
#include <string.h>
#include <exception>
#include <mutex>
#include <functional>
#include <stdexcept>
#include <string>

#include <iostream>

namespace brigid {
  namespace {
    std::once_flag once;

    bool no_full_range_lightuserdata;

    int check_full_range_lightuserdata(lua_State* L) {
      void* ptr = nullptr;
      memset(&ptr, 0xFF, sizeof(ptr));
      lua_pushlightuserdata(L, ptr);
      return 1;
    }

    void bootstrap(lua_State* L) {
      int top = lua_gettop(L);
      lua_pushcfunction(L, check_full_range_lightuserdata);
      no_full_range_lightuserdata = lua_pcall(L, 0, 0, 0) != 0;
      lua_settop(L, top);
    }

    int impl_closure(lua_State* L) {
      int top = lua_gettop(L);
      try {
        if (cxx_function_t function = to_handle<cxx_function_t>(L, lua_upvalueindex(1))) {
          function(L);
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
        }
      } catch (const std::runtime_error& e) {
        lua_settop(L, top);
        lua_pushnil(L);
        lua_pushstring(L, e.what());
        return 2;
      } catch (const std::exception& e) {
        lua_settop(L, top);
        return luaL_error(L, "%s", e.what());
      }
      lua_settop(L, top);
      return luaL_error(L, "attempt to call an invalid upvalue");
    }
  }

  int abs_index(lua_State* L, int index) {
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

  int get_table(lua_State* L, int index) {
#if LUA_VERSION_NUM >= 503
    return lua_gettable(L, index);
#else
    lua_gettable(L, index);
    return lua_type(L, -1);
#endif
  }

  void new_metatable(lua_State* L, const char* name) {
    luaL_newmetatable(L, name);
#if LUA_VERSION_NUM <= 502
    lua_pushstring(L, name);
    lua_setfield(L, -2, "__name");
#endif
  }

  void set_metatable(lua_State* L, const char* name) {
#if LUA_VERSION_NUM >= 502
    luaL_setmetatable(L, name);
#else
    luaL_getmetatable(L, name);
    lua_setmetatable(L, -2);
#endif
  }

  bool is_false(lua_State* L, int index) {
    return lua_isboolean(L, index) && !lua_toboolean(L, index);
  }

  void push(lua_State* L, lua_Integer value) {
    lua_pushinteger(L, value);
  }

  void push(lua_State* L, const char* value) {
    lua_pushstring(L, value);
  }

  void push(lua_State* L, const char* data, size_t size) {
    lua_pushlstring(L, data, size);
  }

  void push(lua_State* L, const std::string& value) {
    lua_pushlstring(L, value.data(), value.size());
  }

  void push(lua_State* L, cxx_function_t value) {
    push_handle(L, value);
    lua_pushcclosure(L, impl_closure, 1);
  }

  void push_handle_impl(lua_State* L, const void* source) {
    if (no_full_range_lightuserdata) {
      static constexpr size_t size = sizeof(source);
      char buffer[size] = {};
      memcpy(buffer, &source, size);
      lua_pushlstring(L, buffer, size);
    } else {
      lua_pushlightuserdata(L, const_cast<void*>(source));
    }
  }

  void push_pointer_impl(lua_State* L, const void* source) {
    if (no_full_range_lightuserdata) {
      static constexpr size_t size = sizeof(source);
      char buffer[size] = {};
      memcpy(buffer, &source, size);
      get_field(L, LUA_REGISTRYINDEX, "brigid.common.decode_pointer");
      lua_pushlstring(L, buffer, size);
      if (lua_pcall(L, 1, 1, 0) != 0) {
        throw BRIGID_LOGIC_ERROR(lua_tostring(L, -1));
      }
    } else {
      lua_pushlightuserdata(L, const_cast<void*>(source));
    }
  }

  void* to_handle_impl(lua_State* L, int index) {
    switch (lua_type(L, index)) {
      case LUA_TSTRING:
        {
          size_t size = 0;
          if (const char* data = lua_tolstring(L, index, &size)) {
            if (size == sizeof(void*)) {
              void* result = nullptr;
              memcpy(&result, data, size);
              return result;
            }
          }
        }
        return nullptr;
      case LUA_TLIGHTUSERDATA:
        return lua_touserdata(L, index);
      default:
        return nullptr;
    }
  }

  stack_guard::stack_guard(lua_State* L)
    : state_(L),
      top_(lua_gettop(L)) {}

  stack_guard::~stack_guard() {
    lua_settop(state_, top_);
  }

  reference::reference()
    : state_(),
      state_ref_(LUA_NOREF),
      ref_(LUA_NOREF) {}

  reference::reference(lua_State* L, int index)
    : state_(),
      state_ref_(LUA_NOREF),
      ref_(LUA_NOREF) {
    state_ = lua_newthread(L);
    state_ref_ = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_pushvalue(L, index);
    ref_ = luaL_ref(L, LUA_REGISTRYINDEX);
  }

  reference::reference(reference&& that)
    : state_(that.state_),
      state_ref_(that.state_ref_),
      ref_(that.ref_) {
    that.reset();
  }

  reference::~reference() {
    unref();
  }

  reference& reference::operator=(reference&& that) {
    if (this != &that) {
      unref();
      state_ = that.state_;
      state_ref_ = that.state_ref_;
      ref_ = that.ref_;
      that.reset();
    }
    return *this;
  }

  lua_State* reference::state() const {
    return state_;
  }

  int reference::get_field(lua_State* L) const {
    return brigid::get_field(L, LUA_REGISTRYINDEX, ref_);
  }

  void reference::unref() {
    if (lua_State* L = state_) {
      luaL_unref(L, LUA_REGISTRYINDEX, state_ref_);
      luaL_unref(L, LUA_REGISTRYINDEX, ref_);
      reset();
    }
  }

  void reference::reset() {
    state_ = nullptr;
    state_ref_ = LUA_NOREF;
    ref_ = LUA_NOREF;
  }

  scope_exit::scope_exit(std::function<void ()> function)
    : function_(function) {}

  scope_exit::~scope_exit() {
    try {
      function_();
    } catch (...) {}
  }

  void initialize_common(lua_State* L) {
    try {
      std::call_once(once, bootstrap, L);
    } catch (const std::exception& e) {
      luaL_error(L, "%s", e.what());
    }

    lua_newtable(L);
    {
      stack_guard guard(L);
      static const char code[] =
      #include "common.lua"
      ;
      if (luaL_loadbuffer(L, code, strlen(code), "=(load)") != 0) {
        throw BRIGID_LOGIC_ERROR(lua_tostring(L, -1));
      }
      lua_pushvalue(L, -2);
      lua_pushboolean(L, no_full_range_lightuserdata);
      if (lua_pcall(L, 2, 0, 0) != 0) {
        throw BRIGID_LOGIC_ERROR(lua_tostring(L, -1));
      }
    }
    {
      if (no_full_range_lightuserdata) {
        get_field(L, -1, "decode_pointer");
        set_field(L, LUA_REGISTRYINDEX, "brigid.common.decode_pointer");
      }

      get_field(L, -1, "is_love2d_data");
      set_field(L, LUA_REGISTRYINDEX, "brigid.common.is_love2d_data");
    }
    lua_pop(L, 1);
  }
}
