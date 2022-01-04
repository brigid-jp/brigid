// vim: syntax=ragel:

// Copyright (c) 2022 <dev@brigid.jp>
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

    hasher* check_hasher(lua_State* L, int arg) {
      return check_udata<hasher>(L, arg, "brigid.hasher");
    }

    void impl_gc(lua_State* L) {
      hasher* self = check_hasher(L, 1);
      self->~hasher();
    }

    void impl_call(lua_State* L) {
      const char* name = luaL_checkstring(L, 2);
      if (!new_hasher(L, name)) {
        luaL_argerror(L, 2, "unsupported hash");
      }
    }

    void impl_get_name(lua_State* L) {
      hasher* self = check_hasher(L, 1);
      lua_pushstring(L, self->get_name());
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
      lua_pop(L, 1);

      decltype(function<impl_call>())::set_metafield(L, -1, "__call");
      decltype(function<impl_get_name>())::set_field(L, -1, "get_name");
      decltype(function<impl_update>())::set_field(L, -1, "update");
      decltype(function<impl_digest>())::set_field(L, -1, "digest");
    }
    lua_setfield(L, -2, "hasher");
  }
}
