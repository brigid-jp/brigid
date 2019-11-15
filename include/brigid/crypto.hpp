// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_CRYPTO_HPP
#define BRIGID_CRYPTO_HPP

#include <stddef.h>
#include <memory>
#include <string>

namespace brigid {
  class encryptor_impl {
  public:
    virtual ~encryptor_impl() = 0;
    virtual size_t block_bytes() const = 0;
    virtual size_t update(const char*, size_t, char*, size_t, bool) = 0;
  };

  std::unique_ptr<encryptor_impl> make_encryptor_impl(const std::string&, const char*, size_t, const char*, size_t);

  class encryptor {
  public:
    encryptor(const std::string&, const char*, size_t, const char*, size_t);
    size_t update(const char*, size_t, char*, size_t, bool);
  private:
    std::unique_ptr<encryptor_impl> impl_;
  };
}

#endif
