// Copyright (c) 2019,2021 <dev@brigid.jp>
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
    class data_writer_t : public abstract_data_t, private noncopyable {
    public:
      data_writer_t()
        : closed_() {}

      virtual bool closed() const {
        return closed_;
      }

      virtual const char* data() const {
        return buffer_.data();
      }

      virtual size_t size() const {
        return buffer_.size();
      }

      void close() {
        closed_ = true;
      }

      void write_self() {
        size_t size = buffer_.size();
        buffer_.resize(size * 2);
        char* data = buffer_.data();
        memcpy(data + size, data, size);
      }

      void write(const char* data, size_t size) {
        size_t position = buffer_.size();
        buffer_.resize(position + size);
        memcpy(buffer_.data() + position, data, size);
      }

    private:
      std::vector<char> buffer_;
      bool closed_;
    };

    data_writer_t* check_data_writer(lua_State* L, int arg, int validate = check_validate_all) {
      data_writer_t* self = check_udata<data_writer_t>(L, arg, "brigid.data_writer");
      if (validate & check_validate_not_closed) {
        if (self->closed()) {
          luaL_error(L, "attempt to use a closed brigid.data_writer");
        }
      }
      return self;
    }

    void impl_gc(lua_State* L) {
      check_data_writer(L, 1, check_validate_none)->~data_writer_t();
    }

    void impl_close(lua_State* L) {
      data_writer_t* self = check_data_writer(L, 1, check_validate_none);
      if (!self->closed()) {
        self->close();
      }
    }
    void impl_call(lua_State* L) {
      new_userdata<data_writer_t>(L, "brigid.data_writer");
    }

    void impl_get_pointer(lua_State* L) {
      data_writer_t* self = check_data_writer(L, 1);
      get_field(L, LUA_REGISTRYINDEX, "brigid.common.decode_pointer");
      push_pointer(L, self->data());
      if (lua_pcall(L, 1, 1, 0) != 0) {
        throw BRIGID_LOGIC_ERROR(lua_tostring(L, -1));
      }
    }

    void impl_get_size(lua_State* L) {
      data_writer_t* self = check_data_writer(L, 1);
      push(L, self->size());
    }

    void impl_get_string(lua_State* L) {
      data_writer_t* self = check_data_writer(L, 1);
      push(L, self->data(), self->size());
    }

    void impl_write(lua_State* L) {
      data_writer_t* self = check_data_writer(L, 1);
      if (self == lua_touserdata(L, 2)) {
        self->write_self();
      } else {
        data_t data = check_data(L, 2);
        self->write(data.data(), data.size());
      }
    }
  }

  void initialize_data_writer(lua_State* L) {
    lua_newtable(L);
    {
      new_metatable(L, "brigid.data_writer");
      lua_pushvalue(L, -2);
      set_field(L, -2, "__index");
      set_field(L, -1, "__gc", impl_gc);
      set_field(L, -1, "__close", impl_close);
      set_field(L, -1, "__len", impl_get_size);
      set_field(L, -1, "__tostring", impl_get_string);
      lua_pop(L, 1);

      set_metafield(L, -1, "__call", impl_call);
      set_field(L, -1, "get_pointer", impl_get_pointer);
      set_field(L, -1, "get_size", impl_get_size);
      set_field(L, -1, "get_string", impl_get_string);
      set_field(L, -1, "close", impl_close);
      set_field(L, -1, "write", impl_write);
    }
    set_field(L, -2, "data_writer");
  }
}
