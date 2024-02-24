// Copyright (c) 2024 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_WRITER_HPP
#define BRIGID_WRITER_HPP

#include "common.hpp"
#include "data.hpp"
#include "error.hpp"

#include <lua.hpp>

#include <math.h>
#include <stdio.h>

#include "write_json_string.hxx"
#include "write_urlencoded.hxx"

namespace brigid {
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

    static void initialize(lua_State* L) {
      decltype(function<write_json_number>())::set_field(L, -1, "write_json_number");
      decltype(function<write_json_string>())::set_field(L, -1, "write_json_string");
      decltype(function<write_urlencoded>())::set_field(L, -1, "write_urlencoded");
    }
  };
}

#endif
