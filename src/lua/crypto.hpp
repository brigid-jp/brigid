// Copyright (c) 2021,2022,2024 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_CRYPTO_HPP
#define BRIGID_CRYPTO_HPP

#include "noncopyable.hpp"
#include "thread_reference.hpp"

#include <lua.hpp>

#include <stddef.h>
#include <vector>

namespace brigid {
  void open_cryptor();
  void open_hasher();

  class cryptor {
  public:
    virtual ~cryptor() = 0;
    void update(const char*, size_t, bool);
    void close();
    bool closed() const;
    bool running() const;

  protected:
    explicit cryptor(thread_reference&&);

  private:
    size_t in_size_;
    size_t out_size_;
    std::vector<char> buffer_;
    thread_reference ref_;
    bool running_;

    void ensure_buffer_size(size_t);

    virtual size_t impl_calculate_buffer_size(size_t) const = 0;
    virtual size_t impl_update(const char*, size_t, char*, size_t, bool) = 0;
  };

  cryptor* new_aes_128_cbc_encryptor(lua_State*, const char*, size_t, const char*, size_t, thread_reference&&);
  cryptor* new_aes_192_cbc_encryptor(lua_State*, const char*, size_t, const char*, size_t, thread_reference&&);
  cryptor* new_aes_256_cbc_encryptor(lua_State*, const char*, size_t, const char*, size_t, thread_reference&&);
  cryptor* new_aes_128_cbc_decryptor(lua_State*, const char*, size_t, const char*, size_t, thread_reference&&);
  cryptor* new_aes_192_cbc_decryptor(lua_State*, const char*, size_t, const char*, size_t, thread_reference&&);
  cryptor* new_aes_256_cbc_decryptor(lua_State*, const char*, size_t, const char*, size_t, thread_reference&&);

  cryptor* new_encryptor(lua_State*, const char*, const char*, size_t, const char*, size_t, thread_reference&&);
  cryptor* new_decryptor(lua_State*, const char*, const char*, size_t, const char*, size_t, thread_reference&&);

  class hasher {
  public:
    virtual ~hasher() = 0;
    virtual void update(const char*, size_t) = 0;
    virtual void digest(lua_State*) = 0;
  };

  hasher* new_sha1_hasher(lua_State*);
  hasher* new_sha256_hasher(lua_State*);
  hasher* new_sha512_hasher(lua_State*);
}

#endif
