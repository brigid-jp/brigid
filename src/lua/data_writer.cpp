// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/error.hpp>
#include <brigid/noncopyable.hpp>
#include "common.hpp"
#include "data.hpp"

#include <lua.hpp>

#include <stddef.h>
#include <string.h>
#include <vector>

namespace brigid {
  namespace {
    class data_writer_t : private noncopyable {
    public:
      explicit data_writer_t()
        : buffer_(),
          closed_() {}

      void write(const char* data, size_t size) {
        size_t position = buffer_.size();
        buffer_.resize(position + size);
        memmove(buffer_.data() + position, data, size);
      }

      const char* data() const {
        return buffer_.data();
      }

      size_t size() const {
        return buffer_.size();
      }

      void close() {
        buffer_.clear();
        closed_ = true;
      }

      bool closed() const {
        return closed_;
      }

    private:
      std::vector<char> buffer_;
      bool closed_;
    };

    data_writer_t* check_data_writer(lua_State* L, int arg, bool validate = true) {
      data_writer_t* self = check_udata<data_writer_t>(L, arg, "brigid.data_writer");
      if (validate) {
        if (self->closed()) {
          luaL_error(L, "attempt to use a closed brigid.data_writer");
        }
      }
      return self;
    }

    void impl_gc(lua_State* L) {
      check_data_writer(L, 1, false)->~data_writer_t();
    }

    void impl_call(lua_State* L) {
      new_userdata<data_writer_t>(L, "brigid.data_writer");
    }

    void impl_write(lua_State* L) {
      data_writer_t* self = check_data_writer(L, 1);
      data_t data = check_data(L, 2);
      self->write(data.data(), data.size());
    }

    void impl_get_string(lua_State* L) {
      data_writer_t* self = check_data_writer(L, 1);
      push(L, self->data(), self->size());
    }

    void impl_get_pointer(lua_State* L) {
      data_writer_t* self = check_data_writer(L, 1);
      get_field(L, LUA_REGISTRYINDEX, "brigid.common.decode_pointer");
      push(L, encode_pointer(self->data()));
      if (lua_pcall(L, 1, 1, 0) != 0) {
        throw BRIGID_ERROR(lua_tostring(L, -1));
      }
    }

    void impl_get_size(lua_State* L) {
      data_writer_t* self = check_data_writer(L, 1);
      push(L, self->size());
    }

    void impl_close(lua_State* L) {
      check_data_writer(L, 1)->close();
    }
  }

  void initialize_data_writer(lua_State* L) {
    lua_newtable(L);
    {
      luaL_newmetatable(L, "brigid.data_writer");
      lua_pushvalue(L, -2);
      set_field(L, -2, "__index");
      set_field(L, -1, "__gc", impl_gc);
      lua_pop(L, 1);

      set_metafield(L, -1, "__call", impl_call);
      set_field(L, -1, "write", impl_write);
      set_field(L, -1, "get_string", impl_get_string);
      set_field(L, -1, "get_pointer", impl_get_pointer);
      set_field(L, -1, "get_size", impl_get_size);
      set_field(L, -1, "close", impl_close);

    }
    set_field(L, -2, "data_writer");
  }
}
