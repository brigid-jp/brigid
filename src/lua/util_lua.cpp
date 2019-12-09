// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "util_lua.hpp"

#include <lua.hpp>

#include <stddef.h>
#include <exception>

namespace brigid {
  namespace lua {
    namespace {
      int impl_closure(lua_State* L) {
        int top = lua_gettop(L);
        try {
          cxx_function_t function = reinterpret_cast<cxx_function_t>(lua_touserdata(L, lua_upvalueindex(1)));
          function(L);
        } catch (const std::exception& e) {
          lua_settop(L, top);
          return luaL_error(L, "caught exception: %s", e.what());
        }
        return lua_gettop(L) - top;
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
      top_saver saver;
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

    void push(lua_State* L, lua_Integer value) {
      lua_pushinteger(L, value);
    }

    void push(lua_State* L, const char* value) {
      lua_pushstring(L, value);
    }

    void push(lua_State* L, const char* data, size_t size) {
      lua_pushlstring(L, data, size);
    }

    void push(lua_State* L, cxx_function_t value) {
      lua_pushlightuserdata(L, reinterpret_cast<void*>(value));
      lua_pushcclosure(L, impl_closure, 1);
    }

    top_saver::top_saver(lua_State* L)
      : state_(L),
        top_(lua_gettop(L)) {}

    top_saver::~top_saver() {
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
      that.state_ = nullptr;
      that.state_ref_ = LUA_NOREF;
      that.ref_ = LUA_NOREF;
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
        that.state_ = nullptr;
        that.state_ref_ = LUA_NOREF;
        that.ref_ = LUA_NOREF;
      }
      return *this;
    }

    lua_State* reference::state() const {
      return state_;
    }

    int reference::get_field(lua_State* L) const {
      return lua::get_field(L, LUA_REGISTRYINDEX, ref_);
    }

    void reference::unref() {
      if (lua_State* L = state_) {
        luaL_unref(L, LUA_REGISTRYINDEX, state_ref_);
        luaL_unref(L, LUA_REGISTRYINDEX, ref_);
        state_ = nullptr;
        state_ref_ = LUA_NOREF;
        ref_ = LUA_NOREF;
      }
    }
  }
}
