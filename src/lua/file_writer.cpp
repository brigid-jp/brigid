// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/error.hpp>
#include <brigid/noncopyable.hpp>
#include <brigid/stdio.hpp>
#include "common.hpp"
#include "data.hpp"

#include <lua.hpp>

#include <stddef.h>
#include <string>

namespace brigid {
  namespace {
    class file_writer_t : private noncopyable {
    public:
      explicit file_writer_t(const std::string& path)
        : handle_(open_file_handle(path, "wb")) {}

      void write(const char* data, size_t size) {
        fwrite(data, 1, size, handle_.get());
      }

      void close() {
        handle_.reset();
      }

      bool closed() const {
        return !handle_;
      }

    private:
      file_handle_t handle_;
    };

    file_writer_t* check_file_writer(lua_State* L, int arg, bool validate = true) {
      file_writer_t* self = check_udata<file_writer_t>(L, arg, "brigid.file_writer");
      if (validate) {
        if (self->closed()) {
          luaL_error(L, "attempt to use a closed brigid.file_writer");
        }
      }
      return self;
    }

    void impl_gc(lua_State* L) {
      check_file_writer(L, 1, false)->~file_writer_t();
    }

    void impl_call(lua_State* L) {
      data_t path = check_data(L, 2);
      new_userdata<file_writer_t>(L, "brigid.file_writer", path.str());
    }

    void impl_write(lua_State* L) {
      file_writer_t* self = check_file_writer(L, 1);
      data_t data = check_data(L, 2);
      self->write(data.data(), data.size());
    }

    void impl_close(lua_State* L) {
      check_file_writer(L, 1)->close();
    }
  }

  void initialize_file_writer(lua_State* L) {
    lua_newtable(L);
    {
      luaL_newmetatable(L, "brigid.file_writer");
      lua_pushvalue(L, -2);
      set_field(L, -2, "__index");
      set_field(L, -1, "__gc", impl_gc);
      lua_pop(L, 1);

      set_metafield(L, -1, "__call", impl_call);
      set_field(L, -1, "write", impl_write);
      set_field(L, -1, "close", impl_close);
    }
    set_field(L, -2, "file_writer");
  }
}
