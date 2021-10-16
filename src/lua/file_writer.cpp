// Copyright (c) 2019,2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/error.hpp>
#include <brigid/noncopyable.hpp>
#include <brigid/stdio.hpp>
#include "common.hpp"
#include "data.hpp"

#include <lua.hpp>

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <system_error>

namespace brigid {
  namespace {
    class file_writer_t : private noncopyable {
    public:
      explicit file_writer_t(const char* path)
        : handle_(open_file_handle(path, "wb")) {}

      bool closed() const {
        return !handle_;
      }

      void close() {
        handle_.reset();
      }

      void write(const char* data, size_t size) {
        if (fwrite(data, 1, size, handle_.get()) != size) {
          int code = errno;
          throw BRIGID_RUNTIME_ERROR(std::generic_category().message(code), make_error_code("error number", code));
        }
      }

      void flush() {
        if (fflush(handle_.get()) != 0) {
          int code = errno;
          throw BRIGID_RUNTIME_ERROR(std::generic_category().message(code), make_error_code("error number", code));
        }
      }

    private:
      file_handle_t handle_;
    };

    file_writer_t* check_file_writer(lua_State* L, int arg, int validate = check_validate_all) {
      file_writer_t* self = check_udata<file_writer_t>(L, arg, "brigid.file_writer");
      if (validate & check_validate_not_closed) {
        if (self->closed()) {
          throw BRIGID_LOGIC_ERROR("attempt to use a closed brigid.file_writer");
        }
      }
      return self;
    }

    void impl_gc(lua_State* L) {
      check_file_writer(L, 1, check_validate_none)->~file_writer_t();
    }

    void impl_close(lua_State* L) {
      file_writer_t* self = check_file_writer(L, 1, check_validate_none);
      if (!self->closed()) {
        self->close();
      }
    }

    void impl_call(lua_State* L) {
      const char* path = luaL_checkstring(L, 2);
      new_userdata<file_writer_t>(L, "brigid.file_writer", path);
    }

    void impl_write(lua_State* L) {
      file_writer_t* self = check_file_writer(L, 1);
      data_t data = check_data(L, 2);
      self->write(data.data(), data.size());
    }

    void impl_flush(lua_State* L) {
      file_writer_t* self = check_file_writer(L, 1);
      self->flush();
    }
  }

  void initialize_file_writer(lua_State* L) {
    lua_newtable(L);
    {
      new_metatable(L, "brigid.file_writer");
      lua_pushvalue(L, -2);
      lua_setfield(L, -2, "__index");
      set_field(L, -1, "__gc", impl_gc);
      set_field(L, -1, "__close", impl_close);
      lua_pop(L, 1);

      set_metafield(L, -1, "__call", impl_call);
      set_field(L, -1, "close", impl_close);
      set_field(L, -1, "write", impl_write);
      set_field(L, -1, "flush", impl_flush);
    }
    lua_setfield(L, -2, "file_writer");
  }
}
