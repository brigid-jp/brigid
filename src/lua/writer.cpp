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
#include <string>

namespace brigid {
  void write_json_string(writer_t*, const data_t&);
  void write_urlencoded(writer_t*, const data_t&);

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

#if LUA_VERSION_NUM >= 502
      int result = 0;
      lua_Number value = lua_tonumberx(L, index, &result);
#else
      lua_Number value = lua_tonumber(L, index);
      int result = value != 0 || lua_isnumber(L, index);
#endif
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

    void write_json_indent(writer_t* self, int indent, int depth) {
      self->write('\n');
      for (int i = 0; i < indent * depth; ++i) {
        self->write(' ');
      }
    }

    void write_json(lua_State*, writer_t*, int, int, int);

    void write_json_table(lua_State* L, writer_t* self, int index, int indent, int depth) {
      {
        stack_guard guard(L);

        lua_rawgeti(L, index, 1);
        if (lua_isnil(L, -1)) {
          if (lua_getmetatable(L, index)) {
            luaL_getmetatable(L, "brigid.json.array");
            if (lua_rawequal(L, -1, -2)) {
              self->write("[]", 2);
              return;
            }
          }
        } else {
          self->write('[');
          if (indent) {
            write_json_indent(self, indent, depth + 1);
          }
          write_json(L, self, guard.top() + 1, indent, depth + 1);
          lua_pop(L, 1);

          for (int i = 2; ; ++i) {
            lua_rawgeti(L, index, i);
            if (lua_isnil(L, -1)) {
              break;
            }
            self->write(',');
            if (indent) {
              write_json_indent(self, indent, depth + 1);
            }
            write_json(L, self, guard.top() + 1, indent, depth + 1);
            lua_pop(L, 1);
          }

          if (indent) {
            write_json_indent(self, indent, depth);
          }
          self->write(']');
          return;
        }
      }

      stack_guard guard(L);

      self->write('{');
      bool first = true;

      lua_pushnil(L);
      while (lua_next(L, index)) {
        if (first) {
          first = false;
        } else {
          self->write(',');
        }
        if (indent) {
          write_json_indent(self, indent, depth + 1);
        }

        if (lua_type(L, -2) == LUA_TSTRING) {
          size_t size = 0;
          if (const char* data = lua_tolstring(L, -2, &size)) {
            write_json_string(self, data_t(data, size));
          } else {
            throw BRIGID_LOGIC_ERROR("string expected");
          }
          self->write(':');
          if (indent) {
            self->write(' ');
          }
          write_json(L, self, guard.top() + 2, indent, depth + 1);
          lua_pop(L, 1);
        } else {
          lua_pushvalue(L, -2);
          if (data_t data = to_data(L, guard.top() + 3)) {
            write_json_string(self, data);
          } else {
            throw BRIGID_LOGIC_ERROR("brigid.data expected");
          }
          self->write(':');
          if (indent) {
            self->write(' ');
          }
          write_json(L, self, guard.top() + 2, indent, depth + 1);
          lua_pop(L, 2);
        }
      }

      if (!first && indent) {
        write_json_indent(self, indent, depth);
      }
      self->write('}');
    }

    void write_json(lua_State* L, writer_t* self, int index, int indent, int depth) {
      switch (lua_type(L, index)) {
        case LUA_TNIL:
          self->write("null", 4);
          return;

        case LUA_TNUMBER:
          write_json_number(L, self, index);
          return;

        case LUA_TBOOLEAN:
          if (lua_toboolean(L, index)) {
            self->write("true", 4);
          } else {
            self->write("false", 5);
          }
          return;

        case LUA_TSTRING:
          {
            size_t size = 0;
            if (const char* data = lua_tolstring(L, index, &size)) {
              write_json_string(self, data_t(data, size));
            } else {
              throw BRIGID_LOGIC_ERROR("string expected");
            }
          }
          return;

        case LUA_TTABLE:
          write_json_table(L, self, index, indent, depth);
          return;

        case LUA_TLIGHTUSERDATA:
          if (!lua_touserdata(L, index)) {
            self->write("null", 4);
            return;
          }
          break;
      }

      if (data_t data = to_data(L, index)) {
        write_json_string(self, data);
      } else {
        throw BRIGID_LOGIC_ERROR("brigid.data expected");
      }
    }

    void impl_write_json_number(lua_State* L) {
      writer_t* self = check_writer(L, 1);
      write_json_number(L, self, 2);
    }

    void impl_write_json_string(lua_State* L) {
      writer_t* self = check_writer(L, 1);
      data_t data = check_data(L, 2);
      write_json_string(self, data);
    }

    void impl_write_json(lua_State* L) {
      writer_t* self = check_writer(L, 1);
      int indent = opt_integer<int>(L, 3, 0);
      write_json(L, self, 2, indent, 0);
    }

    void impl_write_urlencoded(lua_State* L) {
      writer_t* self = check_writer(L, 1);
      data_t data = check_data(L, 2);
      write_urlencoded(self, data);
    }
  }

  writer_t::~writer_t() {}

  void initialize_writer(lua_State* L) {
    decltype(function<impl_write_json_number>())::set_field(L, -1, "write_json_number");
    decltype(function<impl_write_json_string>())::set_field(L, -1, "write_json_string");
    decltype(function<impl_write_json>())::set_field(L, -1, "write_json");
    decltype(function<impl_write_urlencoded>())::set_field(L, -1, "write_urlencoded");
  }
}
