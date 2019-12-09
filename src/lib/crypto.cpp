// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/crypto.hpp>
#include <brigid/error.hpp>

#include <stddef.h>

namespace brigid {
  cryptor::~cryptor() {}

  size_t get_block_size(crypto_cipher cipher) {
    switch (cipher) {
      case crypto_cipher::aes_128_cbc:
      case crypto_cipher::aes_192_cbc:
      case crypto_cipher::aes_256_cbc:
        return 16;
    }
    throw BRIGID_ERROR("unsupported cipher");
  }
}
