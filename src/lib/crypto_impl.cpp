// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "crypto_impl.hpp"
#include "error.hpp"

#include <stddef.h>

namespace brigid {
  void check_cipher(crypto_cipher cipher, size_t key_size, size_t iv_size) {
    size_t aes_key_size = 0;
    size_t aes_block_size = 16;

    switch (cipher) {
      case crypto_cipher::aes_128_cbc:
        aes_key_size = 16;
        break;
      case crypto_cipher::aes_192_cbc:
        aes_key_size = 24;
        break;
      case crypto_cipher::aes_256_cbc:
        aes_key_size = 32;
        break;
      default:
        throw BRIGID_ERROR("unsupported cipher");
    }

    if (key_size != aes_key_size) {
      throw BRIGID_ERROR("invalid key size");
    }
    if (iv_size != aes_block_size) {
      throw BRIGID_ERROR("invalid initialization vector size");
    }
  }
}
