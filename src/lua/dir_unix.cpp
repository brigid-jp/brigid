// Copyright (c) 2024 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "common.hpp"
#include "error.hpp"
#include "function.hpp"
#include "noncopyable.hpp"

#include <lua.hpp>

#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <memory>

namespace brigid {
  namespace {
    using dir_handle_t = std::unique_ptr<DIR, decltype(&closedir)>;

    dir_handle_t make_dir_handle(DIR* handle = nullptr) {
      return dir_handle_t(handle, closedir);
    }

    dir_handle_t open_dir_handle(const char* path) {
      if (dir_handle_t result = make_dir_handle(opendir(path))) {
        return result;
      } else {
        throw BRIGID_SYSTEM_ERROR();
      }
    }

    class dir_t : private noncopyable {
    public:
      explicit dir_t(const char* path)
        : handle_(open_dir_handle(path)) {}

      bool closed() const {
        return !handle_;
      }

      void close() {
        handle_.reset();
      }

      const char* read() {
        if (struct dirent* result = readdir(handle_.get())) {
          return result->d_name;
        } else {
          return nullptr;
        }
      }

    private:
      dir_handle_t handle_;
    };

    dir_t* check_dir(lua_State* L, int arg, int validate = check_validate_all) {
      dir_t* self = check_udata<dir_t>(L, arg, "brigid.dir");
      if (validate & check_validate_not_closed) {
        if (self->closed()) {
          luaL_argerror(L, arg, "attempt to use a closed brigid.dir");
        }
      }
      return self;
    }

    void impl_gc(lua_State* L) {
      check_dir(L, 1, check_validate_none)->~dir_t();
    }

    void impl_close(lua_State* L) {
      dir_t* self = check_dir(L, 1, check_validate_none);
      if (!self->closed()) {
        self->close();
      }
    }

    void impl_call(lua_State* L) {
    }

    void impl_read(lua_State* L) {
      dir_t* self = check_dir(L, 1);
      lua_pushstring(L, self->read());
    }

    void impl_opendir(lua_State* L) {
      const char* path = luaL_checkstring(L, 1);
      new_userdata<dir_t>(L, "brigid.dir", path);
    }

    void impl_mkdir(lua_State* L) {
      const char* path = luaL_checkstring(L, 1);
      mode_t mode = opt_integer(L, 2, 0777);
      if (mkdir(path, mode) == -1) {
        throw BRIGID_SYSTEM_ERROR();
      }
    }

    void impl_rmdir(lua_State* L) {
      const char* path = luaL_checkstring(L, 1);
      if (rmdir(path) == -1) {
        throw BRIGID_SYSTEM_ERROR();
      }
    }
  }

  /*

    next_dir(handle)

    for handle, name in brigid.dir "." do
    end

  */
  void initialize_dir(lua_State* L) {
    lua_newtable(L);
    {
      new_metatable(L, "brigid.dir");
      lua_pushvalue(L, -2);
      lua_setfield(L, -2, "__index");
      decltype(function<impl_gc>())::set_field(L, -1, "__gc");
      decltype(function<impl_close>())::set_field(L, -1, "__close");
      lua_pop(L, 1);

      decltype(function<impl_call>())::set_metafield(L, -1, "__call");
      decltype(function<impl_close>())::set_field(L, -1, "close");
      decltype(function<impl_read>())::set_field(L, -1, "read");
    }
    lua_setfield(L, -2, "dir");

    decltype(function<impl_opendir>())::set_field(L, -1, "opendir");
    decltype(function<impl_mkdir>())::set_field(L, -1, "mkdir");
    decltype(function<impl_rmdir>())::set_field(L, -1, "rmdir");
  }
}
