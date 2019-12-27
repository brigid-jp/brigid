// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/error.hpp>
#include <brigid/noncopyable.hpp>
#include "common.hpp"
#include "cryptor.hpp"
#include "data.hpp"
#include "hasher.hpp"

#include <lua.hpp>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "brigid_zone.hpp"

#define BRIGID_ZONE_SET(v, i) \
  do { \
    zone[(i - 1) * 4 + 0] = static_cast<uint8_t>(0xFF & (v) >> 24); \
    zone[(i - 1) * 4 + 1] = static_cast<uint8_t>(0xFF & (v) >> 16); \
    zone[(i - 1) * 4 + 2] = static_cast<uint8_t>(0xFF & (v) >> 8); \
    zone[(i - 1) * 4 + 3] = static_cast<uint8_t>(0xFF & (v)); \
  } while (false)

namespace brigid {
  namespace {
    uint8_t zone[32];

    void impl_put(lua_State* L) {
      size_t position = check_integer<size_t>(L, 1, 1, 32);
      uint8_t value = check_integer<uint8_t>(L, 2, 0, 255);
      zone[position - 1] = value;
    }

    void impl_decryptor(lua_State* L) {
      crypto_cipher cipher = check_cipher(L, 1);
      crypto_hash hash = check_hash(L, 2);
      data_t salt = check_data(L, 3);
      reference write_cb;

      if (!lua_isnoneornil(L, 4)) {
        write_cb = reference(L, 4);
      }

      std::unique_ptr<hasher> key_hasher = make_hasher(hash);
      key_hasher->update(reinterpret_cast<const char*>(zone), 32);
      key_hasher->update(salt.data(), salt.size());
      std::vector<char> key = key_hasher->digest();
      switch (cipher) {
        case crypto_cipher::aes_128_cbc:
          key.resize(16);
          break;
        case crypto_cipher::aes_192_cbc:
          key.resize(24);
          break;
        case crypto_cipher::aes_256_cbc:
          key.resize(32);
          break;
      }

      std::unique_ptr<hasher> iv_hasher = make_hasher(hash);
      iv_hasher->update(key.data(), key.size());
      iv_hasher->update(reinterpret_cast<const char*>(zone), 32);
      iv_hasher->update(salt.data(), salt.size());
      std::vector<char> iv = iv_hasher->digest();
      iv.resize(16);

      new_decryptor(L, make_decryptor(cipher, key.data(), key.size(), iv.data(), iv.size()), std::move(write_cb));
    }
  }

  void initialize_zone(lua_State* L) {
#if BRIGID_ZONE1+0
    BRIGID_ZONE_SET(BRIGID_ZONE1, 1);
#endif
#if BRIGID_ZONE2+0
    BRIGID_ZONE_SET(BRIGID_ZONE2, 2);
#endif
#if BRIGID_ZONE3+0
    BRIGID_ZONE_SET(BRIGID_ZONE3, 3);
#endif
#if BRIGID_ZONE4+0
    BRIGID_ZONE_SET(BRIGID_ZONE4, 4);
#endif
#if BRIGID_ZONE5+0
    BRIGID_ZONE_SET(BRIGID_ZONE5, 5);
#endif
#if BRIGID_ZONE6+0
    BRIGID_ZONE_SET(BRIGID_ZONE6, 6);
#endif
#if BRIGID_ZONE7+0
    BRIGID_ZONE_SET(BRIGID_ZONE7, 7);
#endif
#if BRIGID_ZONE8+0
    BRIGID_ZONE_SET(BRIGID_ZONE8, 8);
#endif

    lua_newtable(L);
    {
      set_field(L, -1, "put", impl_put);
      set_field(L, -1, "decryptor", impl_decryptor);
    }
    set_field(L, -2, "zone");
  }
}
