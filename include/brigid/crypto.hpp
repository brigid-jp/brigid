// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_CRYPTO_HPP
#define BRIGID_CRYPTO_HPP

#include <stddef.h>
#include <memory>
#include <string>

namespace brigid {
  class cryptor {
  public:
    virtual ~cryptor() = 0;
    virtual size_t update(const char*, size_t, char*, size_t, bool) = 0;
  };

  std::unique_ptr<cryptor> make_encryptor(const std::string&, const char*, size_t, const char*, size_t);
  std::unique_ptr<cryptor> make_decryptor(const std::string&, const char*, size_t, const char*, size_t);
}

#endif
