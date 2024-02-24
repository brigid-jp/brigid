// Copyright (c) 2024 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "common.hpp"
#include "data.hpp"
#include "error.hpp"
#include "function.hpp"
#include "stack_guard.hpp"
#include "writer.hpp"

#include <lua.hpp>

#include <math.h>
#include <stdio.h>

#include "write_json_string.hxx"
#include "write_urlencoded.hxx"

namespace brigid {
/*
  template <class T, T* (*T_check_writer)(lua_State*, int, int)>
  struct writer {
    static void write_json_number(lua_State* L) {
      char buffer[64] = {};

      T* self = T_check_writer(L, 1, check_validate_all);
#if LUA_VERSION_NUM >= 503
      {
        int result = 0;
        lua_Integer value = lua_tointegerx(L, 2, &result);
        if (result) {
          int size = snprintf(buffer, sizeof(buffer), LUA_INTEGER_FMT, value);
          if (size < 0) {
            throw BRIGID_SYSTEM_ERROR();
          }
          self->write(buffer, size);
          return;
        }
      }
#endif
      lua_Number value = luaL_checknumber(L, 2);
      if (!isfinite(value)) {
        throw BRIGID_RUNTIME_ERROR("inf or nan");
      }
      int size = snprintf(buffer, sizeof(buffer), "%.17g", value);
      if (size < 0) {
        throw BRIGID_SYSTEM_ERROR();
      }
      self->write(buffer, size);
    }

    static void write_json_string(lua_State* L) {
      T* self = T_check_writer(L, 1, check_validate_all);
      data_t data = check_data(L, 2);
      impl_write_json_string(self, data);
    }

    static void write_urlencoded(lua_State* L) {
      T* self = T_check_writer(L, 1, check_validate_all);
      data_t data = check_data(L, 2);
      impl_write_urlencoded(self, data);
    }
  };

    inline bool luaX_to_udata_impl(lua_State* L, const char* name) {
      luaL_getmetatable(L, name);
      bool result = lua_rawequal(L, -1, -2);
      lua_pop(L, 1);
      return result;
    }

    template <class T>
    inline T* luaX_to_udata(lua_State* L, int index, const char* name) {
      if (T* data = static_cast<T*>(lua_touserdata(L, index))) {
        if (lua_getmetatable(L, index)) {
          if (!luaX_to_udata_impl(L, name)) {
            data = 0;
          }
          lua_pop(L, 1);
          return data;
        }
      }
      return 0;
    }

    template <class T>
    inline T* luaX_to_udata(lua_State* L, int index, const char* name1, const char* name2) {
      if (T* data = static_cast<T*>(lua_touserdata(L, index))) {
        if (lua_getmetatable(L, index)) {
          if (!(luaX_to_udata_impl(L, name1) || luaX_to_udata_impl(L, name2))) {
            data = 0;
          }
          lua_pop(L, 1);
          return data;
        }
      }
      return 0;
    }

*/

  namespace {
    writer_t* check_writer_impl(lua_State* L, int arg) {
      if (writer_t* self = to_data_writer(L, arg)) {
        return self;
      } else if (writer_t* self = to_file_writer(L, arg)) {
        return self;
      }
      luaL_argerror(L, arg, "brigid.writer expected");
      throw BRIGID_LOGIC_ERROR("unreachable");
    }

    writer_t* check_writer(lua_State* L, int arg) {
      writer_t* self = check_writer_impl(L, arg);
      if (!self->closed()) {
        return self;
      }
      luaL_argerror(L, arg, "attempt to use a closed brigid.writer");
      throw BRIGID_LOGIC_ERROR("unreachable");
    }

    void impl_write_json_number(lua_State* L) {
      char buffer[64] = {};

      writer_t* self = check_writer(L, 1);

#if LUA_VERSION_NUM >= 503
      {
        int result = 0;
        lua_Integer value = lua_tointegerx(L, 2, &result);
        if (result) {
          int size = snprintf(buffer, sizeof(buffer), LUA_INTEGER_FMT, value);
          if (size < 0) {
            throw BRIGID_SYSTEM_ERROR();
          }
          self->write(buffer, size);
          return;
        }
      }
#endif

      lua_Number value = luaL_checknumber(L, 2);
      if (!isfinite(value)) {
        throw BRIGID_RUNTIME_ERROR("inf or nan");
      }
      int size = snprintf(buffer, sizeof(buffer), "%.17g", value);
      if (size < 0) {
        throw BRIGID_SYSTEM_ERROR();
      }
      self->write(buffer, size);
    }

    void impl_write_json_string(lua_State* L) {
      writer_t* self = check_writer(L, 1);
      data_t data = check_data(L, 2);
      impl_write_json_string(self, data);
    }

    void impl_write_urlencoded(lua_State* L) {
      writer_t* self = check_writer(L, 1);
      data_t data = check_data(L, 2);
      impl_write_urlencoded(self, data);
    }
  }

  writer_t::~writer_t() {}

  void initialize_writer(lua_State* L) {
    decltype(function<impl_write_json_number>())::set_field(L, -1, "write_json_number");
    decltype(function<impl_write_json_string>())::set_field(L, -1, "write_json_string");
    decltype(function<impl_write_urlencoded>())::set_field(L, -1, "write_urlencoded");
  }
}
