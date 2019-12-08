// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/crypto.hpp>
#include <brigid/error.hpp>
#include "common.hpp"
#include "util_lua.hpp"

#include <vector>

namespace brigid {
  using namespace lua;

  namespace {
    crypto_cipher check_cipher(lua_State* L, int arg) {
      const auto cipher = luax_check_data(L, arg).to_str();
      if (cipher == "aes-128-cbc") {
        return crypto_cipher::aes_128_cbc;
      } else if (cipher == "aes-192-cbc") {
        return crypto_cipher::aes_192_cbc;
      } else if (cipher == "aes-256-cbc") {
        return crypto_cipher::aes_256_cbc;
      }
      throw BRIGID_ERROR("unsupported cipher");
    }

    void impl_encrypt_string(lua_State* L) {
      const auto cipher = check_cipher(L, 1);
      const auto in = luax_check_data(L, 2);
      const auto key = luax_check_data(L, 3);
      const auto iv = luax_check_data(L, 4);
      const auto encryptor = make_encryptor(cipher, key.data(), key.size(), iv.data(), iv.size());
      std::vector<char> out(in.size() + 16);
      size_t result = encryptor->update(in.data(), in.size(), out.data(), out.size(), true);
      luax_push(L, out.data(), result);
    }

    void impl_decrypt_string(lua_State* L) {
      const auto cipher = check_cipher(L, 1);
      const auto in = luax_check_data(L, 2);
      const auto key = luax_check_data(L, 3);
      const auto iv = luax_check_data(L, 4);
      const auto decryptor = make_decryptor(cipher, key.data(), key.size(), iv.data(), iv.size());
      std::vector<char> out(in.size());
      size_t result = decryptor->update(in.data(), in.size(), out.data(), out.size(), true);
      luax_push(L, out.data(), result);
    }


    // void impl_gc(lua_State* L) {
    //   check_udata<cryptor*>(L, -1, "brigid.cryptor")->~cryptor();
    // }

    // void impl_call(lua_State* L) {
    //   std::unique_ptr<cryptor> cryptor = make_decryptor();
    //   crptyor** self = new_userdata<cryptor*>(L, "brigid.cryptor", cryptor.get());
    //   cryptor.release();
    // }
  }

  void initialize_crypto(lua_State* L) {
    luax_set_field(L, -1, "encrypt_string", impl_encrypt_string);
    luax_set_field(L, -1, "decrypt_string", impl_decrypt_string);

    // lua_newtable(L, "cryptor");
    // {
    //   luaL_newmetatable(L, "brigid.cryptor");
    //   lua_pushvalue(L, -2)
    //   set_field(L, -2, "__index");
    //   set_field(L, -1, "__gc", impl_gc);
    //   lua_pop(L, 1);
    // }
    // set_field(L, -1, "cryptor");

    // brigid.cryptor


  }
}
