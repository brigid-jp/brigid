// vim: syntax=ragel:

// Copyright (c) 2019-2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "common.hpp"
#include "crypto.hpp"
#include "data.hpp"
#include "error.hpp"
#include "function.hpp"
#include "noncopyable.hpp"

#include <lua.hpp>

#include <stddef.h>
#include <memory>
#include <utility>
#include <string>
#include <vector>

namespace brigid {
  namespace {
    %%{
      machine hasher_name_chooser;

      main :=
        ( "sha1\0"
          @{ return new_sha1_hasher(L); }
        | "sha256\0"
          @{ return new_sha256_hasher(L); }
        | "sha512\0"
          @{ return new_sha512_hasher(L); }
        );
      write data noerror nofinal noentry;
    }%%

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#endif

    hasher* new_hasher(lua_State* L, const char* name) {
      int cs = 0;
      %%write init;
      const char* p = name;
      const char* pe = nullptr;
      %%write exec;
      return nullptr;
    }

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

/*
    crypto_hash check_hash(lua_State* L, int arg) {
      size_t size = 0;
      if (const char* data = lua_tolstring(L, arg, &size)) {
        std::string hash(data, size);
        if (hash == "sha1") {
          return crypto_hash::sha1;
        } else if (hash == "sha256") {
          return crypto_hash::sha256;
        } else if (hash == "sha512") {
          return crypto_hash::sha512;
        }
      }
      luaL_argerror(L, arg, "unsupported hash");
      throw BRIGID_LOGIC_ERROR("unreachable");
    }

    class hasher_t : private noncopyable {
    public:
      explicit hasher_t(std::unique_ptr<hasher>&& hasher)
        : hasher_(std::move(hasher)) {}

      void update(const char* data, size_t size) {
        hasher_->update(data, size);
      }

      std::vector<char> digest() {
        return hasher_->digest();
      }

      void close() {
        hasher_ = nullptr;
      }

      bool closed() const {
        return !hasher_;
      }

    private:
      std::unique_ptr<hasher> hasher_;
    };

    hasher_t* check_hasher(lua_State* L, int arg, int validate = check_validate_all) {
      hasher_t* self = check_udata<hasher_t>(L, arg, "brigid.hasher");
      if (validate & check_validate_not_closed) {
        if (self->closed()) {
          luaL_error(L, "attempt to use a closed brigid.hasher");
        }
      }
      return self;
    }
*/

    hasher* check_hasher(lua_State* L, int arg) {
      return check_udata<hasher>(L, arg, "brigid.hasher");
    }

    void impl_gc(lua_State* L) {
      hasher* self = check_hasher(L, 1);
      self->~hasher();
    }

/*
    void impl_close(lua_State* L) {
      hasher_t* self = check_hasher(L, 1, check_validate_none);
      if (!self->closed()) {
        self->close();
      }
    }
*/

    void impl_call(lua_State* L) {
      const char* name = luaL_checkstring(L, 2);
      if (!new_hasher(L, name)) {
        luaL_argerror(L, 2, "unsupported hash");
      }
    }

    void impl_update(lua_State* L) {
      hasher* self = check_hasher(L, 1);
      data_t source = check_data(L, 2);
      self->update(source.data(), source.size());
    }

    void impl_digest(lua_State* L) {
      hasher* self = check_hasher(L, 1);
      std::vector<char> result = self->digest();
      lua_pushlstring(L, result.data(), result.size());
    }
  }

  void initialize_hasher(lua_State* L) {
    try {
      open_hasher();
    } catch (const std::exception& e) {
      luaL_error(L, "%s", e.what());
      return;
    }

    lua_newtable(L);
    {
      new_metatable(L, "brigid.hasher");
      lua_pushvalue(L, -2);
      lua_setfield(L, -2, "__index");
      decltype(function<impl_gc>())::set_field(L, -1, "__gc");
      // decltype(function<impl_close>())::set_field(L, -1, "__close");
      lua_pop(L, 1);

      decltype(function<impl_call>())::set_metafield(L, -1, "__call");
      decltype(function<impl_update>())::set_field(L, -1, "update");
      decltype(function<impl_digest>())::set_field(L, -1, "digest");
      // decltype(function<impl_close>())::set_field(L, -1, "close");
    }
    lua_setfield(L, -2, "hasher");
  }
}
