// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/error.hpp>
#include <brigid/http.hpp>
#include "data.hpp"
#include "util_lua.hpp"
#include "view.hpp"

#include <lua.hpp>

#include <functional>
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
          write_cb_(std::move(write_cb)) {}

      void request(
          const std::string& method,
          const std::string& url,
          const std::map<std::string, std::string>& header,
          http_request_body body,
          const char* data, size_t size) {
        session_->request(method, url, header, body, data, size);
      }

    private:
      std::unique_ptr<http_session> session_;
      reference progress_cb_;
      reference header_cb_;
      reference write_cb_;

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

    http_session_t* check_http_session(lua_State* L, int arg) {
      return check_udata<http_session_t>(L, arg, "brigid.http_session");
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

      {
        stack_guard guard(L);
        if (get_field(L, -1, "progress") != LUA_TNIL) {
          progress_cb = reference(L, -1);
        }
      }

      {
        stack_guard guard(L);
        if (get_field(L, -1, "header") != LUA_TNIL) {
          header_cb = reference(L, -1);
        }
      }

      {
        stack_guard guard(L);
        if (get_field(L, -1, "write") != LUA_TNIL) {
          write_cb = reference(L, -1);
        }
      }

      {
        stack_guard guard(L);
        if (get_field(L, -1, "username") != LUA_TNIL) {
          username = to_data(L, -1).str();
          ++credential;
        }
      }

      {
        stack_guard guard(L);
        if (get_field(L, -1, "password") != LUA_TNIL) {
          password = to_data(L, -1).str();
          ++credential;
        }
      }

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

      {
        stack_guard guard(L);
        if (get_field(L, -1, "method") != LUA_TNIL) {
          method = to_data(L, -1).str();
        }
      }

      {
        stack_guard guard(L);
        if (get_field(L, -1, "url") != LUA_TNIL) {
          url = to_data(L, -1).str();
        }
      }

      {
        stack_guard guard(L);
        if (get_field(L, -1, "header") != LUA_TNIL) {
          int index = abs_index(L, -1);
          lua_pushnil(L);
          while (true) {
            if (!lua_next(L, index)) {
              break;
            }
            lua_pushvalue(L, -2);
            data_t key = to_data(L, -1);
            data_t value = to_data(L, -2);
            header[key.str()] = value.str();
            lua_pop(L, 2);
          }
        }
      }

      if (get_field(L, -1, "data") != LUA_TNIL) {
        body = http_request_body::data;
        data = to_data(L, -1);
      } else {
        lua_pop(L, 1);
        if (get_field(L, -1, "file") != LUA_TNIL) {
          body = http_request_body::file;
          data = to_data(L, -1);
        }
      }

      self->request(method, url, header, body, data.data(), data.size());
      lua_pop(L, 1);
    }
  }

  void initialize_http(lua_State* L) {
    lua_newtable(L);
    {
      stack_guard guard(L);
      luaL_newmetatable(L, "brigid.http_session");
      lua_pushvalue(L, -2);
      set_field(L, -2, "__index");
      set_field(L, -1, "__gc", impl_gc);
    }
    {
      set_metafield(L, -1, "__call", impl_call);
      set_field(L, -1, "request", impl_request);
    }
    set_field(L, -2, "http_session");
  }
}
