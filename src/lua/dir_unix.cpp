// Copyright (c) 2024 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "common.hpp"
#include "error.hpp"
#include "function.hpp"

#include <lua.hpp>

#include <sys/stat.h>
#include <unistd.h>

namespace brigid {
  namespace {
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

  void initialize_dir(lua_State* L) {
    decltype(function<impl_mkdir>())::set_field(L, -1, "mkdir");
    decltype(function<impl_rmdir>())::set_field(L, -1, "rmdir");
  }
}
