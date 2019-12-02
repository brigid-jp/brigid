// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_CRYPTO_HPP
#define BRIGID_CRYPTO_HPP

#include <brigid/noncopyable.hpp>

#include <stddef.h>
#include <memory>

namespace brigid {
  enum class crypto_cipher { aes_128_cbc, aes_192_cbc, aes_256_cbc };

  class crypto_initializer : private noncopyable {
  public:
    crypto_initializer();
    ~crypto_initializer();
  };

  namespace {
    crypto_initializer crypto_initializer_instance;
  }

  class cryptor {
  public:
    virtual ~cryptor() = 0;
    virtual size_t update(const char*, size_t, char*, size_t, bool) = 0;
  };

  std::unique_ptr<cryptor> make_encryptor(crypto_cipher, const char*, size_t, const char*, size_t);
  std::unique_ptr<cryptor> make_decryptor(crypto_cipher, const char*, size_t, const char*, size_t);
}

#endif
