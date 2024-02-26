// Copyright (c) 2024 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_DIR_WINDOWS_HPP
#define BRIGID_DIR_WINDOWS_HPP

#include "error.hpp"
#include "noncopyable.hpp"

#include <lua.hpp>

#include <io.h>
#include <direct.h>
#include <string>

namespace brigid {
  namespace {
    class dir_t : private noncopyable {
    public:
      explicit dir_t(const std::string& path)
        : first_(true),
          handle_(-1),
          data_() {
        handle_ = _findfirst((path + "\\*").c_str(), &data_);
        if (handle_ == -1) {
          throw BRIGID_SYSTEM_ERROR();
        }
      }

      ~dir_t() {
        close();
      }

      bool closed() const {
        return handle_ == -1;
      }

      void close() {
        if (handle_ != -1) {
          _findclose(handle_);
          handle_ = -1;
        }
      }

      const char* read() {
        if (first_) {
          first_ = false;
        } else {
          if (_findnext(handle_, &data_) == -1) {
            bool error = errno != ENOENT;
            close();
            if (error) {
              throw BRIGID_SYSTEM_ERROR();
            } else {
              return nullptr;
            }
          }
        }
        return data_.name;
      }

    private:
      bool first_;
      intptr_t handle_;
      _finddata_t data_;
    };

    void impl_mkdir(lua_State* L) {
      const char* path = luaL_checkstring(L, 1);
      if (_mkdir(path) == -1) {
        throw BRIGID_SYSTEM_ERROR();
      }
    }

    void impl_rmdir(lua_State* L) {
      const char* path = luaL_checkstring(L, 1);
      if (_rmdir(path) == -1) {
        throw BRIGID_SYSTEM_ERROR();
      }
    }
  }
}

#endif
