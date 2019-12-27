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
    static const size_t zone_size = 32;
    uint8_t zone[zone_size];

    std::vector<char> make_key(crypto_cipher cipher, crypto_hash hash, const data_t& salt) {
      std::unique_ptr<hasher> hasher = make_hasher(hash);
      hasher->update(reinterpret_cast<const char*>(zone), zone_size);
      hasher->update(salt.data(), salt.size());
      std::vector<char> result = hasher->digest();
      switch (cipher) {
        case crypto_cipher::aes_128_cbc:
          result.resize(16);
          break;
        case crypto_cipher::aes_192_cbc:
          result.resize(24);
          break;
        case crypto_cipher::aes_256_cbc:
          result.resize(32);
          break;
      }
      return result;
    }

    std::vector<char> make_iv(crypto_cipher cipher, crypto_hash hash, const data_t& salt, const std::vector<char>& key) {
      std::unique_ptr<hasher> hasher = make_hasher(hash);
      hasher->update(key.data(), key.size());
      hasher->update(reinterpret_cast<const char*>(zone), zone_size);
      hasher->update(salt.data(), salt.size());
      std::vector<char> result = hasher->digest();
      switch (cipher) {
        case crypto_cipher::aes_128_cbc:
        case crypto_cipher::aes_192_cbc:
        case crypto_cipher::aes_256_cbc:
          result.resize(16);
          break;
      }
      return result;
    }

    void impl_put(lua_State* L) {
      size_t position = check_integer<size_t>(L, 1, 1, zone_size);
      uint8_t value = check_integer<uint8_t>(L, 2, 0, 255);
      zone[position - 1] = value;
    }

    void impl_sha256(lua_State* L) {
      std::unique_ptr<hasher> hasher = make_hasher(crypto_hash::sha256);
      hasher->update(reinterpret_cast<const char*>(zone), zone_size);
      std::vector<char> result = hasher->digest();
      push(L, result.data(), result.size());
    }

    void impl_sha512(lua_State* L) {
      std::unique_ptr<hasher> hasher = make_hasher(crypto_hash::sha512);
      hasher->update(reinterpret_cast<const char*>(zone), zone_size);
      std::vector<char> result = hasher->digest();
      push(L, result.data(), result.size());
    }

    void impl_decryptor(lua_State* L) {
      crypto_cipher cipher = check_cipher(L, 1);
      crypto_hash hash = check_hash(L, 2);
      data_t salt = check_data(L, 3);
      reference write_cb;

      if (!lua_isnoneornil(L, 4)) {
        write_cb = reference(L, 4);
      }

      std::vector<char> key = make_key(cipher, hash, salt);
      std::vector<char> iv = make_iv(cipher, hash, salt, key);
      new_decryptor(L, make_decryptor(cipher, key.data(), key.size(), iv.data(), iv.size()), std::move(write_cb));
    }

    void impl_load(lua_State* L) {
      crypto_cipher cipher = check_cipher(L, 1);
      crypto_hash hash = check_hash(L, 2);
      data_t source = check_data(L, 3);
      data_t check_hash = to_data(L, 4);

      if (source.size() < 16) {
        luaL_argerror(L, 4, "invalid source");
      }
      if (data_t(source.data(), 8).str() != "Salted__") {
        luaL_argerror(L, 4, "invalid source");
      }

      data_t salt(source.data() + 8, 8);
      std::vector<char> key = make_key(cipher, hash, salt);
      std::vector<char> iv = make_iv(cipher, hash, salt, key);

      std::unique_ptr<cryptor> decryptor = make_decryptor(cipher, key.data(), key.size(), iv.data(), iv.size());
      std::vector<char> buffer(decryptor->calculate_buffer_size(source.size()));
      size_t result = decryptor->update(source.data() + 16, source.size() - 16, buffer.data(), buffer.size(), true);
      buffer.resize(result);

      std::unique_ptr<hasher> check_hasher;
      switch (check_hash.size()) {
        case 32:
          check_hasher = make_hasher(crypto_hash::sha256);
          break;
        case 64:
          check_hasher = make_hasher(crypto_hash::sha512);
          break;
      }
      if (check_hasher) {
        check_hasher->update(buffer.data(), buffer.size());
        std::vector<char> result = check_hasher->digest();
        if (!std::equal(result.begin(), result.end(), check_hash.data())) {
          throw BRIGID_RUNTIME_ERROR("invalid hash");
        }
      }

      if (luaL_loadbuffer(L, buffer.data(), buffer.size(), "=(load)") != 0) {
        throw BRIGID_RUNTIME_ERROR(lua_tostring(L, -1));
      }
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
      set_field(L, -1, "sha256", impl_sha256);
      set_field(L, -1, "sha512", impl_sha512);
      set_field(L, -1, "decryptor", impl_decryptor);
      set_field(L, -1, "load", impl_load);
    }
    set_field(L, -2, "zone");
  }
}
