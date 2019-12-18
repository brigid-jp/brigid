// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/error.hpp>
#include "common.hpp"

#include <lua.hpp>

#include <stddef.h>
#include <string.h>
#include <exception>
#include <string>

namespace brigid {
  namespace {
    int impl_closure(lua_State* L) {
      int top = lua_gettop(L);
      try {
        size_t size = 0;
        const char* data = lua_tolstring(L, lua_upvalueindex(1), &size);
        if (cxx_function_t function = decode_pointer<cxx_function_t>(data, size)) {
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
        lua_pushnil(L),
        push(L, e.what());
        return 2;
      } catch (const std::exception& e) {
        lua_settop(L, top);
        return luaL_error(L, "%s", e.what());
      }
      lua_settop(L, top);
      return luaL_error(L, "attempt to call an invalid upvalue");
    }

    void impl_encode_pointer(lua_State* L) {
      push(L, encode_pointer(lua_touserdata(L, 1)));
    }

    void impl_decode_pointer(lua_State* L) {
      size_t size = 0;
      const char* data = lua_tolstring(L, 1, &size);
      lua_pushlightuserdata(L, decode_pointer<void*>(data, size));
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

  void set_metatable(lua_State* L, const char* name) {
#if LUA_VERSION_NUM >= 502
    luaL_setmetatable(L, name);
#else
    luaL_getmetatable(L, name);
    lua_setmetatable(L, -2);
#endif
  }

  void* test_udata_impl(lua_State* L, int index, const char* name) {
#if LUA_VERSION_NUM >= 502
    return luaL_testudata(L, index, name);
#else
    stack_guard guard(L);
    if (void* data = lua_touserdata(L, index)) {
      if (lua_getmetatable(L, index)) {
        luaL_getmetatable(L, name);
        if (lua_rawequal(L, -1, -2)) {
          return data;
        }
      }
    }
    return nullptr;
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
    push(L, encode_pointer(value));
    lua_pushcclosure(L, impl_closure, 1);
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

  void initialize_common(lua_State* L) {
    lua_newtable(L);
    {
      set_field(L, -1, "encode_pointer", impl_encode_pointer);
      set_field(L, -1, "decode_pointer", impl_decode_pointer);
    }
    {
      stack_guard guard(L);
      static const char code[] =
      #include "common.lua"
      ;
      if (luaL_loadstring(L, code) == 0) {
        lua_pushvalue(L, -2);
        lua_pcall(L, 1, 0, 0);
      }
    }
    {
      get_field(L, -1, "encode_pointer");
      set_field(L, LUA_REGISTRYINDEX, "brigid.common.encode_pointer");
      get_field(L, -1, "decode_pointer");
      set_field(L, LUA_REGISTRYINDEX, "brigid.common.decode_pointer");
      get_field(L, -1, "is_love2d_data");
      set_field(L, LUA_REGISTRYINDEX, "brigid.common.is_love2d_data");
    }
    lua_pop(L, 1);
  }
}
