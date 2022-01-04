// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_CRYPTO_HPP
#define BRIGID_CRYPTO_HPP

#include "noncopyable.hpp"

#include <lua.hpp>

#include <stddef.h>
#include <memory>
#include <vector>

namespace brigid {
  enum class crypto_cipher { aes_128_cbc, aes_192_cbc, aes_256_cbc };
  enum class crypto_hash { sha1, sha256, sha512 };

  class crypto_initializer : private noncopyable {
  public:
    crypto_initializer();
    ~crypto_initializer();
  };

  namespace {
    crypto_initializer crypto_initializer_instance;
  }

  void open_cryptor();
  void open_hasher();

  class cryptor {
  public:
    virtual ~cryptor() = 0;
    virtual size_t calculate_buffer_size(size_t) const = 0;
    virtual size_t update(const char*, size_t, char*, size_t, bool) = 0;
  };

  std::unique_ptr<cryptor> make_encryptor(crypto_cipher, const char*, size_t, const char*, size_t);
  std::unique_ptr<cryptor> make_decryptor(crypto_cipher, const char*, size_t, const char*, size_t);

  class hasher {
  public:
    virtual ~hasher() = 0;
    virtual void update(const char*, size_t) = 0;
    virtual std::vector<char> digest() = 0;
  };

  std::unique_ptr<hasher> make_hasher(crypto_hash);

  class crypto {
  public:
    virtual ~crypto() = 0;
    virtual const char* get_name() const = 0;
    virtual hasher* new_hasher(lua_State* L, const char*) const = 0;
  };
}

#endif
