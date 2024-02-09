// Copyright (c) 2019-2021,2024 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "common.hpp"
#include "data.hpp"
#include "error.hpp"
#include "function.hpp"
#include "http.hpp"
#include "noncopyable.hpp"
#include "scope_exit.hpp"
#include "stack_guard.hpp"
#include "thread_reference.hpp"
#include "view.hpp"

#include <lua.hpp>

#include <stddef.h>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>

namespace brigid {
  using namespace std::placeholders;

  namespace {
    class http_session_t : private noncopyable {
    public:
      http_session_t(
          thread_reference&& ref,
          int progress_cb,
          int header_cb,
          int write_cb,
          bool credential,
          const std::string& username,
          const std::string& password)
        : session_(make_http_session(
            std::bind(&http_session_t::progress_cb, this, _1, _2),
            std::bind(&http_session_t::header_cb, this, _1, _2),
            std::bind(&http_session_t::write_cb, this, _1, _2),
            credential,
            username,
            password)),
          ref_(std::move(ref)),
          progress_cb_(progress_cb),
          header_cb_(header_cb),
          write_cb_(write_cb),
          running_() {}

      bool request(
          const std::string& method,
          const std::string& url,
          const std::map<std::string, std::string>& header,
          http_request_body body,
          const char* data, size_t size) {
        return session_->request(method, url, header, body, data, size);
      }

      void close() {
        session_ = nullptr;
        ref_ = thread_reference();
      }

      bool closed() const {
        return !session_;
      }

      bool running() const {
        return running_;
      }

    private:
      std::unique_ptr<http_session> session_;
      thread_reference ref_;
      int progress_cb_;
      int header_cb_;
      int write_cb_;
      bool running_;

      bool progress_cb(size_t now, size_t total) {
        if (progress_cb_) {
          if (lua_State* L = ref_.get()) {
            stack_guard guard(L);
            lua_pushvalue(L, progress_cb_);
            push_integer(L, now);
            push_integer(L, total);
            running_ = true;
            scope_exit scope_guard([&]() {
              running_ = false;
            });
            if (lua_pcall(L, 2, 1, 0) != 0) {
              throw BRIGID_RUNTIME_ERROR(lua_tostring(L, -1));
            }
            if (is_false(L, -1)) {
              return false;
            }
          }
        }
        return true;
      }

      bool header_cb(int code, const std::map<std::string, std::string>& header) {
        if (header_cb_) {
          if (lua_State* L = ref_.get()) {
            stack_guard guard(L);
            lua_pushvalue(L, header_cb_);
            push_integer(L, code);
            lua_newtable(L);
            for (const auto& field : header) {
              lua_pushlstring(L, field.first.data(), field.first.size());
              lua_pushlstring(L, field.second.data(), field.second.size());
              lua_rawset(L, -3);
            }
            running_ = true;
            scope_exit scope_guard([&]() {
              running_ = false;
            });
            if (lua_pcall(L, 2, 1, 0) != 0) {
              throw BRIGID_RUNTIME_ERROR(lua_tostring(L, -1));
            }
            if (is_false(L, -1)) {
              return false;
            }
          }
        }
        return true;
      }

      bool write_cb(const char* data, size_t size) {
        if (write_cb_) {
          if (lua_State* L = ref_.get()) {
            stack_guard guard(L);
            lua_pushvalue(L, write_cb_);
            view_t* view = new_view(L, data, size);
            running_ = true;
            scope_exit scope_guard([&]() {
              running_ = false;
              view->close();
            });
            if (lua_pcall(L, 1, 1, 0) != 0) {
              throw BRIGID_RUNTIME_ERROR(lua_tostring(L, -1));
            }
            if (is_false(L, -1)) {
              return false;
            }
          }
        }
        return true;
      }
    };

    http_session_t* check_http_session(lua_State* L, int arg, int validate = check_validate_all) {
      http_session_t* self = check_udata<http_session_t>(L, arg, "brigid.http_session");
      if (validate & check_validate_not_closed) {
        if (self->closed()) {
          luaL_error(L, "attempt to use a closed brigid.http_session");
        }
      }
      if (validate & check_validate_not_running) {
        if (self->running()) {
          luaL_error(L, "attempt to use a running brigid.http_session");
        }
      }
      return self;
    }

    void impl_gc(lua_State* L) {
      check_http_session(L, 1, check_validate_none)->~http_session_t();
    }

    void impl_close(lua_State* L) {
      http_session_t* self = check_http_session(L, 1, check_validate_not_running);
      if (!self->closed()) {
        self->close();
      }
    }

    void impl_call(lua_State* L) {
      luaL_checktype(L, 2, LUA_TTABLE);

      thread_reference ref;
      int ref_index = 0;
      int progress_cb = 0;
      int header_cb = 0;
      int write_cb = 0;
      int credential = 0;
      std::string username;
      std::string password;

      if (get_field(L, 2, "progress") != LUA_TNIL) {
        if (!ref) {
          ref = thread_reference(L);
        }
        lua_pushvalue(L, -1);
        lua_xmove(L, ref.get(), 1);
        progress_cb = ++ref_index;
      }
      lua_pop(L, 1);

      if (get_field(L, 2, "header") != LUA_TNIL) {
        if (!ref) {
          ref = thread_reference(L);
        }
        lua_pushvalue(L, -1);
        lua_xmove(L, ref.get(), 1);
        header_cb = ++ref_index;
      }
      lua_pop(L, 1);

      if (get_field(L, 2, "write") != LUA_TNIL) {
        if (!ref) {
          ref = thread_reference(L);
        }
        lua_pushvalue(L, -1);
        lua_xmove(L, ref.get(), 1);
        write_cb = ++ref_index;
      }
      lua_pop(L, 1);

      if (get_field(L, 2, "username") != LUA_TNIL) {
        size_t size = 0;
        if (const char* data = lua_tolstring(L, -1, &size)) {
          ++credential;
          username.assign(data, size);
        }
      }
      lua_pop(L, 1);

      if (get_field(L, 2, "password") != LUA_TNIL) {
        size_t size = 0;
        if (const char* data = lua_tolstring(L, -1, &size)) {
          ++credential;
          password.assign(data, size);
        }
      }
      lua_pop(L, 1);

      new_userdata<http_session_t>(L, "brigid.http_session",
          std::move(ref),
          progress_cb,
          header_cb,
          write_cb,
          credential == 2,
          username,
          password);
    }

    void impl_request(lua_State* L) {
      http_session_t* self = check_http_session(L, 1);
      luaL_checktype(L, 2, LUA_TTABLE);

      std::string method = "GET";
      std::string url;
      std::map<std::string, std::string> header;
      http_request_body body = http_request_body::none;
      data_t data;

      if (get_field(L, 2, "method") != LUA_TNIL) {
        size_t size = 0;
        if (const char* data = lua_tolstring(L, -1, &size)) {
          method.assign(data, size);
        }
      }
      lua_pop(L, 1);

      if (get_field(L, 2, "url") != LUA_TNIL) {
        size_t size = 0;
        if (const char* data = lua_tolstring(L, -1, &size)) {
          url.assign(data, size);
        }
      }
      lua_pop(L, 1);

      if (get_field(L, 2, "header") == LUA_TTABLE) {
        int index = abs_index(L, -1);
        lua_pushnil(L);
        while (lua_next(L, index)) {
          lua_pushvalue(L, -2);
          size_t name_size = 0;
          if (const char* name_data = lua_tolstring(L, -1, &name_size)) {
            size_t value_size = 0;
            if (const char* value_data = lua_tolstring(L, -2, &value_size)) {
              header.emplace(std::string(name_data, name_size), std::string(value_data, value_size));
            }
          }
          lua_pop(L, 2);
        }
      }
      lua_pop(L, 1);

      if (get_field(L, 2, "data") != LUA_TNIL) {
        body = http_request_body::data;
        data = to_data(L, -1);
      }
      if (get_field(L, 2, "file") != LUA_TNIL) {
        body = http_request_body::file;
        data = to_data(L, -1);
      }

      bool result = self->request(method, url, header, body, data.data(), data.size());
      lua_pop(L, 2);

      if (!result) {
        lua_pushnil(L);
        lua_pushstring(L, "canceled");
      }
    }
  }

  http_session::~http_session() {}

  void initialize_http(lua_State* L) {
    try {
      open_http();
    } catch (const std::exception& e) {
      luaL_error(L, "%s", e.what());
      return;
    }

    lua_newtable(L);
    {
      new_metatable(L, "brigid.http_session");
      lua_pushvalue(L, -2);
      lua_setfield(L, -2, "__index");
      decltype(function<impl_gc>())::set_field(L, -1, "__gc");
      decltype(function<impl_close>())::set_field(L, -1, "__close");
      lua_pop(L, 1);

      decltype(function<impl_call>())::set_metafield(L, -1, "__call");
      decltype(function<impl_request>())::set_field(L, -1, "request");
      decltype(function<impl_close>())::set_field(L, -1, "close");
    }
    lua_setfield(L, -2, "http_session");
  }
}
