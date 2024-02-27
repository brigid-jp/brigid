// Copyright (c) 2019-2022,2024 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "common.hpp"
#include "crypto.hpp"
#include "data.hpp"
#include "error.hpp"
#include "function.hpp"
#include "thread_reference.hpp"

#include <lua.hpp>

#include <stddef.h>
#include <utility>

namespace brigid {
  namespace {
    cryptor* check_cryptor(lua_State* L, int arg, int validate = check_validate_all) {
      cryptor* self = check_udata<cryptor>(L, arg, "brigid.cryptor");
      if (validate & check_validate_not_closed) {
        if (self->closed()) {
          luaL_argerror(L, arg, "attempt to use a closed brigid.cryptor");
        }
      }
      if (validate & check_validate_not_running) {
        if (self->running()) {
          luaL_argerror(L, arg, "attempt to use a running brigid.cryptor");
        }
      }
      return self;
    }

    void impl_gc(lua_State* L) {
      check_cryptor(L, 1, check_validate_none)->~cryptor();
    }

    void impl_close(lua_State* L) {
      cryptor* self = check_cryptor(L, 1, check_validate_not_running);
      if (!self->closed()) {
        self->close();
      }
    }

    void impl_update(lua_State* L) {
      cryptor* self = check_cryptor(L, 1);
      data_t source = check_data(L, 2);
      bool padding = lua_toboolean(L, 3) ? true : false;
      self->update(source.data(), source.size(), padding);
    }

    void impl_encryptor(lua_State* L) {
      const char* name = luaL_checkstring(L, 1);
      data_t key = check_data(L, 2);
      data_t iv = check_data(L, 3);

      thread_reference ref;
      if (!lua_isnoneornil(L, 4)) {
        ref = thread_reference(L);
        lua_pushvalue(L, 4);
        lua_xmove(L, ref.get(), 1);
      }

      new_encryptor(L, name, key.data(), key.size(), iv.data(), iv.size(), std::move(ref));
    }

    void impl_decryptor(lua_State* L) {
      const char* name = luaL_checkstring(L, 1);
      data_t key = check_data(L, 2);
      data_t iv = check_data(L, 3);

      thread_reference ref;
      if (!lua_isnoneornil(L, 4)) {
        ref = thread_reference(L);
        lua_pushvalue(L, 4);
        lua_xmove(L, ref.get(), 1);
      }

      new_decryptor(L, name, key.data(), key.size(), iv.data(), iv.size(), std::move(ref));
    }
  }

  void initialize_cryptor(lua_State* L) {
    try {
      open_cryptor();
    } catch (const std::exception& e) {
      luaL_error(L, "%s", e.what());
      return;
    }

    lua_newtable(L);
    {
      new_metatable(L, "brigid.cryptor");
      lua_pushvalue(L, -2);
      lua_setfield(L, -2, "__index");
      decltype(function<impl_gc>())::set_field(L, -1, "__gc");
      decltype(function<impl_close>())::set_field(L, -1, "__close");
      lua_pop(L, 1);

      decltype(function<impl_update>())::set_field(L, -1, "update");
      decltype(function<impl_close>())::set_field(L, -1, "close");
    }
    lua_setfield(L, -2, "cryptor");

    decltype(function<impl_encryptor>())::set_field(L, -1, "encryptor");
    decltype(function<impl_decryptor>())::set_field(L, -1, "decryptor");
  }
}
