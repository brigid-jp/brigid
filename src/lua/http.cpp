// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/error.hpp>
#include <brigid/http.hpp>
#include <brigid/noncopyable.hpp>
#include "data.hpp"
#include "util_lua.hpp"
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
  using namespace lua;

  namespace {
    class http_session_t : private noncopyable {
    public:
      http_session_t(
          reference&& progress_cb,
          reference&& header_cb,
          reference&& write_cb,
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
          progress_cb_(std::move(progress_cb)),
          header_cb_(std::move(header_cb)),
          write_cb_(std::move(write_cb)),
          code_(-1) {}

      void request(
          const std::string& method,
          const std::string& url,
          const std::map<std::string, std::string>& header,
          http_request_body body,
          const char* data, size_t size) {
        session_->request(method, url, header, body, data, size);
      }

      int code() const {
        return code_;
      }

      const std::map<std::string, std::string>& header() const {
        return header_;
      }

      void close() {
        session_ = nullptr;
        progress_cb_ = reference();
        header_cb_ = reference();
        write_cb_ = reference();
        code_ = -1;
        header_.clear();
      }

      bool closed() const {
        return !session_;
      }

    private:
      std::unique_ptr<http_session> session_;
      reference progress_cb_;
      reference header_cb_;
      reference write_cb_;
      int code_;
      std::map<std::string, std::string> header_;

      bool progress_cb(size_t now, size_t total) {
        if (lua_State* L = progress_cb_.state()) {
          stack_guard guard(L);
          progress_cb_.get_field(L);
          push(L, now);
          push(L, total);
          if (lua_pcall(L, 2, 1, 0) != 0) {
            throw BRIGID_ERROR(lua_tostring(L, -1));
          }
          if (is_false(L, -1)) {
            return false;
          }
        }
        return true;
      }

      bool header_cb(int code, const std::map<std::string, std::string>& header) {
        code_ = code;
        header_ = header;
        if (lua_State* L = header_cb_.state()) {
          stack_guard guard(L);
          header_cb_.get_field(L);
          push(L, code);
          lua_newtable(L);
          for (const auto& field : header) {
            set_field(L, -1, field.first, field.second);
          }
          if (lua_pcall(L, 2, 1, 0) != 0) {
            throw BRIGID_ERROR(lua_tostring(L, -1));
          }
          if (is_false(L, -1)) {
            return false;
          }
        }
        return true;
      }

      bool write_cb(const char* data, size_t size) {
        if (lua_State* L = write_cb_.state()) {
          stack_guard guard(L);
          write_cb_.get_field(L);
          view_guard vguard(new_view(L, data, size));
          if (lua_pcall(L, 1, 1, 0) != 0) {
            throw BRIGID_ERROR(lua_tostring(L, -1));
          }
          if (is_false(L, -1)) {
            return false;
          }
        }
        return true;
      }
    };

    http_session_t* check_http_session(lua_State* L, int arg, bool check_closed = true) {
      http_session_t* self = check_udata<http_session_t>(L, arg, "brigid.http_session");
      if (check_closed) {
        if (self->closed()) {
          luaL_error(L, "attempt to use a closed brigid.http_session");
        }
      }
      return self;
    }

    void impl_gc(lua_State* L) {
      check_http_session(L, -1)->~http_session_t();
    }

    void impl_call(lua_State* L) {
      luaL_checktype(L, 2, LUA_TTABLE);

      reference progress_cb;
      reference header_cb;
      reference write_cb;
      int credential = 0;
      std::string username;
      std::string password;

      if (get_field(L, 2, "progress") != LUA_TNIL) {
        progress_cb = reference(L, -1);
      }
      if (get_field(L, 2, "header") != LUA_TNIL) {
        header_cb = reference(L, -1);
      }
      if (get_field(L, 2, "write") != LUA_TNIL) {
        write_cb = reference(L, -1);
      }
      if (get_field(L, 2, "username") != LUA_TNIL) {
        ++credential;
        username = to_data(L, -1).str();
      }
      if (get_field(L, 2, "password") != LUA_TNIL) {
        ++credential;
        password = to_data(L, -1).str();
      }
      lua_pop(L, 5);

      new_userdata<http_session_t>(L, "brigid.http_session",
          std::move(progress_cb),
          std::move(header_cb),
          std::move(write_cb),
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
        method = to_data(L, -1).str();
      }
      if (get_field(L, 2, "url") != LUA_TNIL) {
        url = to_data(L, -1).str();
      }
      if (get_field(L, 2, "header") != LUA_TTABLE) {
        int index = abs_index(L, -1);
        lua_pushnil(L);
        while (true) {
          if (!lua_next(L, index)) {
            break;
          }
          lua_pushvalue(L, -2);
          header[to_data(L, -1).str()] = to_data(L, -2).str();
          lua_pop(L, 2);
        }
      }
      if (get_field(L, 2, "data") != LUA_TNIL) {
        body = http_request_body::data;
        data = to_data(L, -1);
      }
      if (get_field(L, 2, "file") != LUA_TNIL) {
        body = http_request_body::file;
        data = to_data(L, -1);
      }

      self->request(method, url, header, body, data.data(), data.size());
      lua_pop(L, 5);

      push(L, self->code());
      lua_newtable(L);
      for (const auto& field : self->header()) {
        set_field(L, -1, field.first, field.second);
      }
    }

    void impl_close(lua_State* L) {
      check_http_session(L, 1)->close();
    }
  }

  void initialize_http(lua_State* L) {
    lua_newtable(L);
    {
      luaL_newmetatable(L, "brigid.http_session");
      lua_pushvalue(L, -2);
      set_field(L, -2, "__index");
      set_field(L, -1, "__gc", impl_gc);
      lua_pop(L, 1);

      set_metafield(L, -1, "__call", impl_call);
      set_field(L, -1, "request", impl_request);
      set_field(L, -1, "close", impl_close);
    }
    set_field(L, -2, "http_session");
  }
}
