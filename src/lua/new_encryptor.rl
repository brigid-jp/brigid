// vim: syntax=ragel:

// Copyright (c) 2022 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "crypto.hpp"

#include <lua.hpp>

#include <stddef.h>

namespace brigid {
  namespace {
    %%{
      machine encryptor_name_chooser;

      main :=
        ( "aes-128-cbc\0"
          @{ return new_aes_128_cbc_encryptor(L, key_data, key_size, iv_data, iv_size, std::move(ref)); }
        | "aes-192-cbc\0"
          @{ return new_aes_192_cbc_encryptor(L, key_data, key_size, iv_data, iv_size, std::move(ref)); }
        | "aes-256-cbc\0"
          @{ return new_aes_256_cbc_encryptor(L, key_data, key_size, iv_data, iv_size, std::move(ref)); }
        );
      write data noerror nofinal noentry;
    }%%
  }

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#endif

  cryptor* new_encryptor(lua_State* L, const char* name, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size, thread_reference&& ref) {
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
}
