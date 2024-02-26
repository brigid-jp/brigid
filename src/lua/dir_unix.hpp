// Copyright (c) 2024 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_DIR_UNIX_HPP
#define BRIGID_DIR_UNIX_HPP

#include "common.hpp"
#include "error.hpp"
#include "noncopyable.hpp"

#include <lua.hpp>

#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
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
        errno = 0;
        if (struct dirent* result = readdir(handle_.get())) {
          return result->d_name;
        } else {
          bool error = errno != 0;
          close();
          if (error) {
            throw BRIGID_SYSTEM_ERROR();
          } else {
            return nullptr;
          }
        }
      }

    private:
      dir_handle_t handle_;
    };

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
}

#endif
