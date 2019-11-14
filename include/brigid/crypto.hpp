// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_CRYPTO_HPP
#define BRIGID_CRYPTO_HPP

#include <memory>

namespace brigid {
  class decryptor;
  class encryptor;

  class crypto {
  public:
    virtual ~crypto() = 0;
    virtual const char* backend() const = 0;
    virtual std::unique_ptr<decryptor> create_decryptor(const char*) = 0;
    virtual std::unique_ptr<encryptor> create_encryptor(const char*) = 0;
  };

  std::unique_ptr<crypto> create_crypto(const char* = 0);
}

#endif
