// Copyright (c) 2024 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "common.hpp"
#include "data.hpp"
#include "error.hpp"
#include "function.hpp"
#include "writer.hpp"

#include <lua.hpp>

#include <math.h>
#include <stdio.h>

#include "write_json_string.hxx"
#include "write_urlencoded.hxx"

namespace brigid {
  namespace {
    writer_t* check_writer_impl(lua_State* L, int arg) {
      if (writer_t* self = to_writer_data_writer(L, arg)) {
        return self;
      } else if (writer_t* self = to_writer_file_writer(L, arg)) {
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

    template <class T>
    int snprintf_wrapper(char* buffer, size_t size, const char* format, T value) {
#ifdef _MSC_VER
        return _snprintf_s(buffer, size, size - 1, format, value);
#else
        return snprintf(buffer, size, format, value);
#endif
    }

    void write_json_number(lua_State* L, writer_t* self, int index) {
      char buffer[64] = {};

#if LUA_VERSION_NUM >= 503
      {
        int result = 0;
        lua_Integer value = lua_tointegerx(L, index, &result);
        if (result) {
          int size = snprintf_wrapper(buffer, sizeof(buffer), LUA_INTEGER_FMT, value);
          if (size < 0) {
            throw BRIGID_SYSTEM_ERROR();
          }
          self->write(buffer, size);
          return;
        }
      }
#endif

      int result = 0;
      lua_Number value = lua_tonumberx(L, index, &result);
      if (!result) {
        throw BRIGID_LOGIC_ERROR("number expected");
      }
      if (!isfinite(value)) {
        throw BRIGID_LOGIC_ERROR("inf or nan");
      }

      if (value == 0) { // check for both zero and minus zero
        self->write('0');
        return;
      }

      int size = snprintf_wrapper(buffer, sizeof(buffer), "%.17g", value);
      if (size < 0) {
        throw BRIGID_SYSTEM_ERROR();
      }
      self->write(buffer, size);
    }

    void write_json_string(lua_State* L, writer_t* self, int index) {
      data_t data = to_data(L, index);
      if (!data.data()) {
        throw BRIGID_LOGIC_ERROR("brigid.data expected");
      }
      write_json_string_impl(self, data);
    }

    void impl_write_json_number(lua_State* L) {
      writer_t* self = check_writer(L, 1);
      write_json_number(L, self, 2);
    }

    void impl_write_json_string(lua_State* L) {
      writer_t* self = check_writer(L, 1);
      write_json_string(L, self, 2);
    }

    void impl_write_urlencoded(lua_State* L) {
      writer_t* self = check_writer(L, 1);
      data_t data = check_data(L, 2);
      write_urlencoded_impl(self, data);
    }
  }

  writer_t::~writer_t() {}

  void initialize_writer(lua_State* L) {
    decltype(function<impl_write_json_number>())::set_field(L, -1, "write_json_number");
    decltype(function<impl_write_json_string>())::set_field(L, -1, "write_json_string");
    decltype(function<impl_write_urlencoded>())::set_field(L, -1, "write_urlencoded");
  }
}
