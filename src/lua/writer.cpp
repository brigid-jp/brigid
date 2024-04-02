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
#include <stddef.h>
#include <stdio.h>
#include <algorithm>
#include <vector>

namespace brigid {
  void write_json_string(writer_t*, const char* data, size_t size);
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

    class json_key_t {
    public:
      json_key_t(const char* data, size_t size) : data_(data), size_(size) {}

      const char* data() const {
        return data_;
      }

      std::size_t size() const {
        return size_;
      }

      bool operator<(const json_key_t& that) const {
        return std::lexicographical_compare(data_, data_ + size_, that.data_, that.data_ + that.size_);
      }

    private:
      const char* data_;
      size_t size_;
    };

    using json_keys_t = std::vector<json_key_t>;

    void write_json(lua_State*, writer_t*, int, int, int, json_keys_t*);

    bool write_json_array(lua_State* L, writer_t* self, int index, int indent, int depth, json_keys_t* keys) {
      stack_guard guard(L);

#if LUA_VERSION_NUM >= 502
      size_t size = lua_rawlen(L, index);
#else
      size_t size = lua_objlen(L, index);
#endif
      if (size == 0) {
        if (lua_getmetatable(L, index)) {
          luaL_getmetatable(L, "brigid.json.array");
          if (lua_rawequal(L, -1, -2)) {
            self->write("[]", 2);
            return true;
          }
        }
        return false;
      }

      self->write('[');
      if (indent) {
        write_json_indent(self, indent, depth + 1);
      }
      lua_rawgeti(L, index, 1);
      write_json(L, self, guard.top() + 1, indent, depth + 1, keys);
      lua_pop(L, 1);

      for (size_t i = 2; i <= size; ++i) {
        self->write(',');
        if (indent) {
          write_json_indent(self, indent, depth + 1);
        }
        lua_rawgeti(L, index, i);
        write_json(L, self, guard.top() + 1, indent, depth + 1, keys);
        lua_pop(L, 1);
      }

      if (indent) {
        write_json_indent(self, indent, depth);
      }
      self->write(']');
      return true;
    }

    void write_json_object(lua_State* L, writer_t* self, int index, int indent, int depth, json_keys_t* keys) {
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

        if (lua_type(L, guard.top() + 1) == LUA_TSTRING) {
          size_t size = 0;
          if (const char* data = lua_tolstring(L, guard.top() + 1, &size)) {
            write_json_string(self, data, size);
          } else {
            throw BRIGID_LOGIC_ERROR("string expected");
          }
          self->write(':');
          if (indent) {
            self->write(' ');
          }
          write_json(L, self, guard.top() + 2, indent, depth + 1, keys);
          lua_pop(L, 1);
        } else {
          // 数値が文字列に変換される場合を考慮してコピーをスタックに積む。
          lua_pushvalue(L, guard.top() + 1);
          if (data_t data = to_data(L, guard.top() + 3)) {
            write_json_string(self, data.data(), data.size());
          } else {
            throw BRIGID_LOGIC_ERROR("brigid.data expected");
          }
          self->write(':');
          if (indent) {
            self->write(' ');
          }
          write_json(L, self, guard.top() + 2, indent, depth + 1, keys);
          lua_pop(L, 2);
        }
      }

      if (!first && indent) {
        write_json_indent(self, indent, depth);
      }
      self->write('}');
    }

    void write_json_object_sort_keys(lua_State* L, writer_t* self, int index, int indent, int depth, json_keys_t* keys) {
      stack_guard guard(L);

      self->write('{');
      bool first = true;
      keys->clear();

      lua_pushnil(L);
      while (lua_next(L, index)) {
        if (lua_type(L, guard.top() + 1) == LUA_TSTRING) {
          size_t size = 0;
          if (const char* data = lua_tolstring(L, guard.top() + 1, &size)) {
            keys->emplace_back(data, size);
          } else {
            throw BRIGID_LOGIC_ERROR("string expected");
          }
          lua_pop(L, 1);
        } else {
          // 文字列キー以外は出力してしまう
          if (first) {
            first = false;
          } else {
            self->write(',');
          }
          if (indent) {
            write_json_indent(self, indent, depth + 1);
          }

          // 数値が文字列に変換される場合を考慮してコピーをスタックに積む。
          lua_pushvalue(L, guard.top() + 1);
          if (data_t data = to_data(L, guard.top() + 3)) {
            write_json_string(self, data.data(), data.size());
          } else {
            throw BRIGID_LOGIC_ERROR("brigid.data expected");
          }
          self->write(':');
          if (indent) {
            self->write(' ');
          }
          write_json(L, self, guard.top() + 2, indent, depth + 1, keys);
          lua_pop(L, 2);
        }
      }

      std::sort(keys->begin(), keys->end());

      for (const auto& key : *keys) {
        if (first) {
          first = false;
        } else {
          self->write(',');
        }
        if (indent) {
          write_json_indent(self, indent, depth + 1);
        }

        write_json_string(self, key.data(), key.size());
        self->write(':');
        if (indent) {
          self->write(' ');
        }

        lua_pushlstring(L, key.data(), key.size());
        lua_gettable(L, index);
        write_json(L, self, guard.top() + 1, indent, depth + 1, keys);
        lua_pop(L, 1);
      }

      if (!first && indent) {
        write_json_indent(self, indent, depth);
      }
      self->write('}');
    }

    void write_json_table(lua_State* L, writer_t* self, int index, int indent, int depth, json_keys_t* keys) {
      if (write_json_array(L, self, index, indent, depth, keys)) {
        return;
      }
      if (keys) {
        write_json_object_sort_keys(L, self, index, indent, depth, keys);
      } else {
        write_json_object(L, self, index, indent, depth, keys);
      }
    }

    void write_json(lua_State* L, writer_t* self, int index, int indent, int depth, json_keys_t* keys) {
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
              write_json_string(self, data, size);
            } else {
              throw BRIGID_LOGIC_ERROR("string expected");
            }
          }
          return;

        case LUA_TTABLE:
          write_json_table(L, self, index, indent, depth, keys);
          return;

        case LUA_TLIGHTUSERDATA:
          if (!lua_touserdata(L, index)) {
            self->write("null", 4);
            return;
          }
          break;
      }

      if (data_t data = to_data(L, index)) {
        write_json_string(self, data.data(), data.size());
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
      write_json_string(self, data.data(), data.size());
    }

    void impl_write_json(lua_State* L) {
      writer_t* self = check_writer(L, 1);
      int indent = opt_integer<int>(L, 3, 0);
      bool sort_keys = lua_toboolean(L, 4);

      if (sort_keys) {
        json_keys_t keys;
        write_json(L, self, 2, indent, 0, &keys);
      } else {
        write_json(L, self, 2, indent, 0, nullptr);
      }
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
